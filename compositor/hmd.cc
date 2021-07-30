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

  head_to_view_projection_left_ = ProjectionMatrix() * HeadToViewMatrix(vr::Eye_Left);
  head_to_view_projection_right_ = ProjectionMatrix() * HeadToViewMatrix(vr::Eye_Right);

  return true;
}

void HMD::Shutdown()
{
  if (vr_system_) {
    vr::VR_Shutdown();
    vr_system_ = NULL;
  }
}

void HMD::Submit(Eye *left_eye, Eye *right_eye)
{
  vr::Texture_t leftEyeTexture = {
      (void *)(uintptr_t)left_eye->resolve_texture_id(),  //
      vr::TextureType_OpenGL,                             //
      vr::ColorSpace_Gamma                                //
  };
  vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);

  vr::Texture_t rightEyeTexture = {
      (void *)(uintptr_t)right_eye->resolve_texture_id(),  //
      vr::TextureType_OpenGL,                              //
      vr::ColorSpace_Gamma                                 //
  };
  vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
}

void HMD::UpdateHeadPose()
{
  if (!vr_system_) return;

  vr::VRCompositor()->WaitGetPoses(tracked_device_pose_list_, vr::k_unMaxTrackedDeviceCount, NULL, 0);

  const uint32_t hmdIndex = vr::k_unTrackedDeviceIndex_Hmd;

  if (tracked_device_pose_list_[hmdIndex].bPoseIsValid) {
    head_pose_ = ConvertSteamVRMatrixToMatrix(tracked_device_pose_list_[hmdIndex].mDeviceToAbsoluteTracking);
    head_pose_.invert();
  }
}

Matrix4 HMD::ViewProjectionMatrix(HmdEye hmd_eye)
{
  Matrix4 viewProjection;
  if (hmd_eye == kLeftEye) {
    viewProjection = head_to_view_projection_left_ * head_pose_;
  } else {
    viewProjection = head_to_view_projection_right_ * head_pose_;
  }
  return viewProjection;
}

Matrix4 HMD::ConvertSteamVRMatrixToMatrix(vr::HmdMatrix34_t &pose)
{
  return Matrix4(                                     //
      pose.m[0][0], pose.m[1][0], pose.m[2][0], 0.0,  //
      pose.m[0][1], pose.m[1][1], pose.m[2][1], 0.0,  //
      pose.m[0][2], pose.m[1][2], pose.m[2][2], 0.0,  //
      pose.m[0][3], pose.m[1][3], pose.m[2][3], 1.0f  //
  );
}

Matrix4 HMD::ProjectionMatrix()
{
  if (!vr_system_) return Matrix4();

  float far = 1000.0f;
  float near = 0.1f;
  float e = -2 * (far * near) / (far - near);
  float f = (far + near) / (far - near);

  return Matrix4(  //
      1, 0, 0, 0,  //
      0, 1, 0, 0,  //
      0, 0, f, 1,  //
      0, 0, e, 0   //
  );
}

Matrix4 HMD::HeadToViewMatrix(vr::Hmd_Eye hmd_eye)
{
  if (!vr_system_) return Matrix4();

  vr::HmdMatrix34_t mat = vr_system_->GetEyeToHeadTransform(hmd_eye);

  Matrix4 eyeToHead(                               //
      mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0,  //
      mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0,  //
      mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0,  //
      mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f  //
  );

  return eyeToHead.invert();
}
