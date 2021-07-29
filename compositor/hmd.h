#include <openvr/openvr.h>
#include <z11/vectors.h>

#include "eye.h"
#include "shader.h"

struct VertexDataScene {
  Vector3 position;
  Vector2 texCoord;
};

class HMD
{
 public:
  bool Init();
  void UpdateHeadPose();
  void Submit(Eye *left_eye, Eye *right_eye);
  void Shutdown();
  enum EyeDirection {
    kLeftEye = vr::Eye_Left,
    kRightEye = vr::Eye_Right,
  };
  Matrix4 ViewProjectionMatrix(EyeDirection eye_direction);
  uint32_t display_width();
  uint32_t display_height();

 private:
  uint32_t display_width_;
  uint32_t display_height_;
  vr::IVRSystem *vr_system_;
  vr::TrackedDevicePose_t tracked_device_pose_list_[vr::k_unMaxTrackedDeviceCount];
  Matrix4 projection_left_;
  Matrix4 projection_right_;
  Matrix4 head_to_view_left_;
  Matrix4 head_to_view_right_;
  Matrix4 head_pose_;

 private:
  Matrix4 ConvertSteamVRMatrixToMatrix4(vr::HmdMatrix34_t &pose);
  Matrix4 ProjectionMatrix(vr::Hmd_Eye hmd_eye);
  Matrix4 HeadToViewMatrix(vr::Hmd_Eye hmd_eye);
};

inline uint32_t HMD::display_width() { return display_width_; }
inline uint32_t HMD::display_height() { return display_height_; }
