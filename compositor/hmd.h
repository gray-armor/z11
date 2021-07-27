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
  bool InitGL();
  void UpdateHeadPose();
  void Submit(Eye *left_eye, Eye *right_eye);
  void Shutdown();
  Matrix4 ViewProjectionMatrix(vr::Hmd_Eye hmd_eye);
  uint32_t display_width();
  uint32_t display_height();

 private:
  uint32_t display_width_;
  uint32_t display_height_;
  vr::IVRSystem *vr_system_;
  Shader shader_;
  unsigned int vertex_count_;
  GLuint vertex_array_object_;
  GLuint vertex_buffer_;
  Matrix4 head_pose_;
  vr::TrackedDevicePose_t tracked_device_pose_list_[vr::k_unMaxTrackedDeviceCount];
  Matrix4 projection_left_;
  Matrix4 projection_right_;
  Matrix4 head_to_eye_left_;
  Matrix4 head_to_eye_right_;

 private:
  Matrix4 ConvertSteamVRMatrixToMatrix4(vr::HmdMatrix34_t &pose);
  Matrix4 ProjectionMatrix(vr::Hmd_Eye hmd_eye);
  Matrix4 HeadToEyeMatrix(vr::Hmd_Eye hmd_eye);
};

inline uint32_t HMD::display_width() { return display_width_; }
inline uint32_t HMD::display_height() { return display_height_; }
