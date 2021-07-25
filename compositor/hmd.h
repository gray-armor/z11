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
  bool InitGL(Eye *left_eye, Eye *right_eye);
  void Draw(Eye *left_eye, Eye *right_eye);
  void Shutdown();
  void UpdateHeadPose();

 private:
  vr::IVRSystem *vr_system_;
  Shader shader_;
  uint32_t display_width_;
  uint32_t display_height_;
  unsigned int vertex_count_;
  GLuint texture_;
  GLuint matrix_location_;
  GLuint vertex_array_object_;
  GLuint vertex_buffer_;
  Matrix4 head_pose_;
  vr::TrackedDevicePose_t tracked_device_pose_list_[vr::k_unMaxTrackedDeviceCount];

 private:
  Matrix4 ConvertSteamVRMatrixToMatrix4(vr::HmdMatrix34_t &pose);
  Matrix4 GetCurrentViewProjectionMatrix(Eye *eye);
};
