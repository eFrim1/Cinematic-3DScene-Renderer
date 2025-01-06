#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace gps {
    
    enum MOVE_DIRECTION {MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT};
    
    class Camera {

    public:
        //Camera constructor
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);

        glm::vec3 getCameraTarget();
        glm::vec3 getCameraPosition();
        glm::vec3 getCameraDirection();

        glm::mat4 getViewMatrix();

        void move(MOVE_DIRECTION direction, float speed);

        void rotate(float pitch, float yaw);

        void setTarget(glm::vec3 vec1);

        void setPosition(glm::vec3 vec1);

        glm::vec3 cameraPosition;

    private:

        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
        void updateCameraVectors();
    };
}

#endif /* Camera_hpp */
