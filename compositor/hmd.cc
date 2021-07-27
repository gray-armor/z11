#include "hmd.h"

#include <openvr/openvr.h>

#include <vector>

bool HMD::Init()
{
  vr::EVRInitError initError = vr::VRInitError_None;
  vr_system_ = vr::VR_Init(&initError, vr::VRApplication_Scene);
  if (initError != vr::VRInitError_None) {
    fprintf(stdout, "Unable to init OpenVR!\n");
    return false;
  }

  if (!vr::VRCompositor()) {
    fprintf(stdout, "Unable to init VR Compositor!\n");
    return false;
  }

  vr_system_->GetRecommendedRenderTargetSize(&display_width_, &display_height_);
  fprintf(stdout, "HMD display width : %d\n", display_width_);
  fprintf(stdout, "HMD display height: %d\n", display_height_);

  projection_left_ = GetProjectionEye(vr::Eye_Left);
  projection_right_ = GetProjectionEye(vr::Eye_Right);
  eye_pose_left_ = GetPoseEye(vr::Eye_Left);
  eye_pose_right_ = GetPoseEye(vr::Eye_Right);

  return true;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
bool HMD::InitGL()
{
  if (shader_.Init(  //
          "Scene",

          // vertex shader
          "#version 410\n"
          "uniform mat4 matrix;\n"
          "layout(location = 0) in vec4 position;\n"
          "layout(location = 1) in vec2 v2UVcoordsIn;\n"
          "layout(location = 2) in vec3 v3NormalIn;\n"
          "out vec2 v2UVcoords;\n"
          "void main()\n"
          "{\n"
          "	v2UVcoords = v2UVcoordsIn;\n"
          "	gl_Position = matrix * position;\n"
          "}\n",

          // fragment shader
          "#version 410 core\n"
          "uniform sampler2D mytexture;\n"
          "in vec2 v2UVcoords;\n"
          "out vec4 outputColor;\n"
          "void main()\n"
          "{\n"
          // "   outputColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
          "   outputColor = texture(mytexture, v2UVcoords);\n"
          "}\n") == false)
    return false;

  // glGenVertexArrays(1, &vertex_array_object_);
  // glBindVertexArray(vertex_array_object_);

  // // glGenBuffers(1, &vertex_buffer_);
  // // glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  // // glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(),
  // &vertdataarray[0], GL_STATIC_DRAW);

  // glEnableVertexAttribArray(0);
  // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexDataScene),
  //                       (void *)offsetof(VertexDataScene, position));

  // glEnableVertexAttribArray(1);
  // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataScene),
  //                       (void *)offsetof(VertexDataScene, texCoord));

  // glBindVertexArray(0);

  // glDisableVertexAttribArray(0);
  // glDisableVertexAttribArray(1);

  std::vector<float> vertdataarray;

  glGenVertexArrays(1, &vertex_array_object_);
  glBindVertexArray(vertex_array_object_);
  glGenBuffers(1, &vertex_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STATIC_DRAW);

  GLsizei stride = sizeof(VertexDataScene);
  uintptr_t offset = 0;

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

  offset += sizeof(Vector3);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

  glBindVertexArray(0);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

  return true;
}

void HMD::Draw(Eye *left_eye, Eye *right_eye)
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, left_eye->framebuffer_id());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, left_eye->copy_framebuffer_id());
  glBlitFramebuffer(0, 0, left_eye->width(), left_eye->height(), 0, 0, left_eye->width(), left_eye->height(),
                    GL_COLOR_BUFFER_BIT, GL_LINEAR);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, right_eye->framebuffer_id());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, right_eye->copy_framebuffer_id());
  glBlitFramebuffer(0, 0, right_eye->width(), right_eye->height(), 0, 0, right_eye->width(),
                    right_eye->height(), GL_COLOR_BUFFER_BIT, GL_LINEAR);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void HMD::Shutdown()
{
  if (vr_system_) {
    vr::VR_Shutdown();
    vr_system_ = NULL;
  }

  // TODO: Delete Framebuffers, Textures, Vertex Arrays
}

void HMD::Submit(Eye *left_eye, Eye *right_eye)
{
  // fprintf(stdout, "submit\n");
  vr::Texture_t leftEyeTexture = {
      (void *)(uintptr_t)left_eye->copy_texture_id(),  //
      vr::TextureType_OpenGL,                          //
      vr::ColorSpace_Gamma                             //
  };
  vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);

  vr::Texture_t rightEyeTexture = {
      (void *)(uintptr_t)right_eye->copy_texture_id(),  //
      vr::TextureType_OpenGL,                           //
      vr::ColorSpace_Gamma                              //
  };
  vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
}

void HMD::UpdateHeadPose()
{
  if (!vr_system_) return;

  vr::VRCompositor()->WaitGetPoses(tracked_device_pose_list_, vr::k_unMaxTrackedDeviceCount, NULL, 0);

  const uint32_t hmdIndex = vr::k_unTrackedDeviceIndex_Hmd;

  if (tracked_device_pose_list_[hmdIndex].bPoseIsValid) {
    head_pose_ = ConvertSteamVRMatrixToMatrix4(tracked_device_pose_list_[hmdIndex].mDeviceToAbsoluteTracking);
    head_pose_.invert();
  }
}

Matrix4 HMD::GetCurrentViewProjectionMatrix(vr::Hmd_Eye hmd_eye)
{
  Matrix4 viewProjection;
  if (hmd_eye == vr::Eye_Left) {
    viewProjection = projection_left_ * eye_pose_left_ * head_pose_;
  } else {
    viewProjection = projection_right_ * eye_pose_right_ * head_pose_;
  }
  return viewProjection;
}

Matrix4 HMD::ConvertSteamVRMatrixToMatrix4(vr::HmdMatrix34_t &pose)
{
  Matrix4 mat(pose.m[0][0], pose.m[1][0], pose.m[2][0], 0.0,    //
              pose.m[0][1], pose.m[1][1], pose.m[2][1], 0.0,    //
              pose.m[0][2], pose.m[1][2], pose.m[2][2], 0.0,    //
              pose.m[0][3], pose.m[1][3], pose.m[2][3], 1.0f);  //
  return mat;
}

Matrix4 HMD::GetProjectionEye(vr::Hmd_Eye hmd_eye)
{
  if (!vr_system_) return Matrix4();

  // クリッピングの範囲.投影行列を
  float nearClip = 0.1;
  float farClip = 200.0;

  vr::HmdMatrix44_t mat = vr_system_->GetProjectionMatrix(hmd_eye, nearClip, farClip);

  return Matrix4(                                          //
      mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],  //
      mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],  //
      mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],  //
      mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]   //
  );
}

Matrix4 HMD::GetPoseEye(vr::Hmd_Eye hmd_eye)
{
  if (!vr_system_) return Matrix4();

  vr::HmdMatrix34_t matEye = vr_system_->GetEyeToHeadTransform(hmd_eye);
  Matrix4 matrixObj(                                        //
      matEye.m[0][0], matEye.m[1][0], matEye.m[2][0], 0.0,  //
      matEye.m[0][1], matEye.m[1][1], matEye.m[2][1], 0.0,  //
      matEye.m[0][2], matEye.m[1][2], matEye.m[2][2], 0.0,  //
      matEye.m[0][3], matEye.m[1][3], matEye.m[2][3], 1.0f  //
  );

  return matrixObj.invert();
}
