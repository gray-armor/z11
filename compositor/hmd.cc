#include "hmd.h"

#include <openvr/openvr.h>

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
  // TODO: sceneとobjectの描画、どのタイミングでやる？ Initで背景の設定, Drawでオブジェクト
  if (shader_.Init(  //
          "Scene",

          // vertex shader
          "#version 410\n"
          "uniform mat4 model;\n"
          "layout(location = 0) in vec4 position;\n"
          "layout(location = 1) in vec2 v2UVcoordsIn;\n"
          "layout(location = 2) in vec3 v3NormalIn;\n"
          "out vec2 v2UVcoords;\n"
          "void main()\n"
          "{\n"
          "	v2UVcoords = v2UVcoordsIn;\n"
          "	gl_Position = model * position;\n"
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
  matrix_location_ = glGetUniformLocation(shader_.id(), "model");
  if (matrix_location_ == (GLuint)-1) {
    fprintf(stdout, "Unable to find matrix uniform in scene shader\n");
    return false;
  }

  glGenVertexArrays(1, &vertex_array_object_);
  glBindVertexArray(vertex_array_object_);

  // glGenBuffers(1, &vertex_buffer_);
  // glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexDataScene),
                        (void *)offsetof(VertexDataScene, position));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataScene),
                        (void *)offsetof(VertexDataScene, texCoord));

  glBindVertexArray(0);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

  return true;
}

void HMD::Draw(Eye *left_eye, Eye *right_eye)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  glUseProgram(shader_.id());
  glUniformMatrix4fv(matrix_location_, 1, GL_FALSE, GetCurrentViewProjectionMatrix(left_eye).get());
  glBindVertexArray(vertex_array_object_);
  glBindTexture(GL_TEXTURE_2D, texture_);
  glDrawArrays(GL_TRIANGLES, 0, vertex_count_);

  glUniformMatrix4fv(matrix_location_, 1, GL_FALSE, GetCurrentViewProjectionMatrix(right_eye).get());
  glBindVertexArray(vertex_array_object_);
  glBindTexture(GL_TEXTURE_2D, texture_);
  glDrawArrays(GL_TRIANGLES, 0, vertex_count_);
  glUseProgram(0);
}

void HMD::Shutdown()
{
  if (vr_system_) {
    vr::VR_Shutdown();
    vr_system_ = NULL;
  }
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
