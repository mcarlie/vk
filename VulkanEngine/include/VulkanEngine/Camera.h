#ifndef CAMERA_H
#define CAMERA_H

#include <VulkanEngine/SceneObject.h>

#include <Eigen/Eigen>
#include <vulkan/vulkan.hpp>

namespace VulkanEngine {

  /// Represents a camera in the scene.
  class Camera : public SceneObject {

  public:

    /// Constructor.
    /// \param _position The position of the Camera.
    /// \param _look_at What position the Camera should be oriented towards initially.
    /// \param _up_vector The up direction of the Camera.
    /// \param _z_near The distance from the Camera's position to the near plane.
    /// \param _z_far The distance from the Camera's position to the far plane.
    /// \param _fov The field of view of the Camera.
    Camera(
      Eigen::Vector3f _look_at = { 0.0f, 0.0f, 0.0f },
      Eigen::Vector3f _up_vector = { 0.0f, 1.0f, 0.0f },
      float _z_near = 0.1f,
      float _z_far = 1000.0f,
      float _fov = 45.0f,
      uint32_t _width = 800,
      uint32_t _height = 600 );

    /// Destructor.
    virtual ~Camera();

    void setLookAt( const Eigen::Vector3f& _look_at );

    /// \return The projection matrix of the Camera.
    const Eigen::Matrix4f getPerspectiveProjectionMatrix();

    /// \return The view matrix of the Camera.
    const Eigen::Matrix4f getViewMatrix();

  protected:

    /// Update the camera. Updates the projection and view matrix in \c scene_state with the camera's values.
    /// \param scene_state Represents the current state of the scene.
    virtual void updateCallback( SceneState& scene_state );

  private:

    /// What position the Camera is oriented towards.
    Eigen::Vector3f look_at;

    /// The up direction of the Camera.
    Eigen::Vector3f up_vector;

    /// The distance from the Camera's position to the near plane.
    float z_near;

    /// The distance from the Camera's position to the far plane.
    float z_far;

    /// The field of view of the Camera.
    float fov;

    /// The current width of the Camera.
    uint32_t width;
    
    /// The current height of the Camera.
    uint32_t height;

  };

}

#endif /* CAMERA_H */
