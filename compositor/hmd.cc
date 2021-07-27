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

  return true;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
bool HMD::InitGL(Eye *left_eye, Eye *right_eye)
{
  // create framebuffer for left eye copy
  glGenFramebuffers(1, &left_copy_framebuffer_id_);
  glBindFramebuffer(GL_FRAMEBUFFER, left_copy_framebuffer_id_);

  glGenTextures(1, &left_copy_texture_id_);
  glBindTexture(GL_TEXTURE_2D, left_copy_texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, left_eye->width(), left_eye->height(), 0, GL_RGBA,
               GL_UNSIGNED_BYTE, nullptr);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, left_copy_texture_id_, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // create framebuffer for right eye copy
  glGenFramebuffers(1, &right_copy_framebuffer_id_);
  glBindFramebuffer(GL_FRAMEBUFFER, right_copy_framebuffer_id_);

  glGenTextures(1, &right_copy_texture_id_);
  glBindTexture(GL_TEXTURE_2D, right_copy_texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, left_eye->width(), left_eye->height(), 0, GL_RGBA,
               GL_UNSIGNED_BYTE, nullptr);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, right_copy_texture_id_, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
  // // glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0],
  // GL_STATIC_DRAW);

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
  // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // VR空間の背景色を設定。
  // glEnable(GL_MULTISAMPLE);

  // copy left eye
  glBindFramebuffer(GL_READ_FRAMEBUFFER, left_eye->framebuffer_id());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, left_copy_framebuffer_id_);
  glBlitFramebuffer(0, 0, left_eye->width(), left_eye->height(), 0, 0, left_eye->width(), left_eye->height(),
                    GL_COLOR_BUFFER_BIT, GL_LINEAR);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  // copy right eye
  glBindFramebuffer(GL_READ_FRAMEBUFFER, right_eye->framebuffer_id());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, right_copy_framebuffer_id_);
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

void HMD::Submit()
{
  // fprintf(stdout, "submit\n");
  vr::Texture_t leftEyeTexture = {(void *)(uintptr_t)left_copy_texture_id_, vr::TextureType_OpenGL,
                                  vr::ColorSpace_Gamma};
  vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);

  vr::Texture_t rightEyeTexture = {(void *)(uintptr_t)right_copy_texture_id_, vr::TextureType_OpenGL,
                                   vr::ColorSpace_Gamma};
  vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
}

void HMD::UpdateHeadPose()
{
  if (vr_system_) return;

  vr::VRCompositor()->WaitGetPoses(tracked_device_pose_list_, vr::k_unMaxTrackedDeviceCount, NULL, 0);

  const uint32_t hmdIndex = vr::k_unTrackedDeviceIndex_Hmd;

  if (tracked_device_pose_list_[hmdIndex].bPoseIsValid) {
    head_pose_ = ConvertSteamVRMatrixToMatrix4(tracked_device_pose_list_[hmdIndex].mDeviceToAbsoluteTracking);
    head_pose_.invert();
  }
}

Matrix4 HMD::GetCurrentViewProjectionMatrix(Eye *eye)
{
  Matrix4 viewProjection = eye->projection() * head_pose_;
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
