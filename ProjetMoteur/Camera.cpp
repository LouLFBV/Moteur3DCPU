#include "Camera.h"
#include <algorithm>
#include <cmath>

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch) : position(startPosition), worldUp(startUp), yaw(startYaw), pitch(startPitch), front(0.0f, 0.0f, -1.0f)
{
    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

void Camera::ProcessKeyboard(CameraDirection direction, float deltaTime)
{
    const float velocity = moveSpeed * deltaTime;

    switch (direction) {
        case CameraDirection::FORWARD:
            position += front * velocity;
            break;

        case CameraDirection::BACKWARD:
            position -= front * velocity;
            break;

        case CameraDirection::LEFT:
            position -= right * velocity;
            break;

        case CameraDirection::RIGHT:
            position += right * velocity;
            break;

        case CameraDirection::UP:
            position += worldUp * velocity;
            break;

        case CameraDirection::DOWN:
            position -= worldUp * velocity;
            break;
    }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constrainPitch){
        pitch = std::clamp(pitch, -89.0f, 89.0f);
    }

    UpdateCameraVectors();
}

void Camera::UpdateCameraVectors()
{
    const float yawRadians = glm::radians(yaw);
    const float pitchRadians = glm::radians(pitch);

    glm::vec3 newFront;

    newFront.x = std::cos(yawRadians) * std::cos(pitchRadians);
    newFront.y = std::sin(pitchRadians);
    newFront.z = std::sin(yawRadians) * std::cos(pitchRadians);

    front = glm::normalize(newFront);

    right = glm::normalize(glm::cross(front, worldUp));

    up = glm::normalize(glm::cross(right, front));
}
