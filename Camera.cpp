#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUp));
        //cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {

        if (direction == MOVE_BACKWARD)
        {
            cameraPosition = cameraPosition - speed * cameraFrontDirection;
        }
        if (direction == MOVE_FORWARD)
        {
            cameraPosition = cameraPosition + speed * cameraFrontDirection;
        }
        if (direction == MOVE_LEFT)
        {
            cameraPosition = cameraPosition - speed * cameraRightDirection;
        }
        if (direction == MOVE_RIGHT)
        {
            cameraPosition = cameraPosition + speed * (cameraRightDirection);
        }

    }

    void Camera::updateCameraVectors() {
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        //cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        cameraFrontDirection = glm::normalize(front);
        updateCameraVectors();
    }

    void Camera::setTarget(glm::vec3 vec1) {
        cameraFrontDirection = vec1;
    }

    void Camera::setPosition(glm::vec3 vec1) {
        cameraPosition = vec1;
    }

    glm::vec3 Camera::getCameraTarget()
    {
        return cameraTarget;
    }

    glm::vec3 Camera::getCameraDirection()
    {
        return cameraFrontDirection;
    }

    glm::vec3 Camera::getCameraPosition()
    {
        return cameraPosition;
    }


}