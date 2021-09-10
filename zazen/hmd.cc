#include "hmd.h"

#include <openvr/openvr.h>

#include <vector>

bool Hmd::Init()
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

  head_to_view_projection_left_ =
      ProjectionMatrix(vr::Eye_Left) * HeadToViewMatrix(vr::Eye_Left);
  head_to_view_projection_right_ =
      ProjectionMatrix(vr::Eye_Right) * HeadToViewMatrix(vr::Eye_Right);
  right_handed_to_left_coord_system_ =
      ConvertRightToLeftHandedCoordSystemMatrix();

  return true;
}

void Hmd::Shutdown()
{
  if (vr_system_) {
    vr::VR_Shutdown();
    vr_system_ = NULL;
  }
}

void Hmd::Submit(Eye *left_eye, Eye *right_eye)
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

void Hmd::UpdateHeadPose()
{
  if (!vr_system_) return;

  vr::VRCompositor()->WaitGetPoses(tracked_device_pose_list_,
                                   vr::k_unMaxTrackedDeviceCount, NULL, 0);

  const uint32_t hmdIndex = vr::k_unTrackedDeviceIndex_Hmd;

  if (tracked_device_pose_list_[hmdIndex].bPoseIsValid) {
    head_pose_ = ConvertSteamVRMatrixToMatrix(
        tracked_device_pose_list_[hmdIndex].mDeviceToAbsoluteTracking);
    head_pose_.invert();
  }
}

Matrix4 Hmd::ViewProjectionMatrix(HmdEye hmd_eye)
{
  Matrix4 viewProjection;
  if (hmd_eye == kLeftEye) {
    viewProjection = head_to_view_projection_left_ * head_pose_ *
                     right_handed_to_left_coord_system_;
  } else {
    viewProjection = head_to_view_projection_right_ * head_pose_ *
                     right_handed_to_left_coord_system_;
  }
  return viewProjection;
}

Matrix4 Hmd::ConvertSteamVRMatrixToMatrix(vr::HmdMatrix34_t &pose)
{
  Matrix4 mat(pose.m[0][0], pose.m[1][0], pose.m[2][0], 0.0,    //
              pose.m[0][1], pose.m[1][1], pose.m[2][1], 0.0,    //
              pose.m[0][2], pose.m[1][2], pose.m[2][2], 0.0,    //
              pose.m[0][3], pose.m[1][3], pose.m[2][3], 1.0f);  //
  return mat;
}

Matrix4 Hmd::ProjectionMatrix(vr::Hmd_Eye hmd_eye)
{
  if (!vr_system_) return Matrix4();

  float nearClip = 0.1f;
  float farClip = 200.0f;

  vr::HmdMatrix44_t mat =
      vr_system_->GetProjectionMatrix(hmd_eye, nearClip, farClip);

  return Matrix4(mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],  //
                 mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],  //
                 mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],  //
                 mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]   //
  );
}

Matrix4 Hmd::HeadToViewMatrix(vr::Hmd_Eye hmd_eye)
{
  if (!vr_system_) return Matrix4();

  vr::HmdMatrix34_t mat = vr_system_->GetEyeToHeadTransform(hmd_eye);

  Matrix4 eyeToHead(mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0,  //
                    mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0,  //
                    mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0,  //
                    mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f  //
  );

  return eyeToHead.invert();
}

// TODO: Use OpenGL API if exists;
Matrix4 Hmd::ConvertRightToLeftHandedCoordSystemMatrix()
{
  return Matrix4(   //
      1, 0, 0, 0,   //
      0, 1, 0, 0,   //
      0, 0, -1, 0,  //
      0, 0, 0, 1    //
  );
}
