#include "Camera.h"


Camera::Camera(glm::vec3 pos, glm::vec3 lookAt, float fov, int width, int height)
{

    position = pos;
    worldUp = glm::vec3(0, -1, 0);
    glm::vec3 dir = glm::normalize(lookAt - position);
    pitch = glm::degrees(asin(dir.y));
    yaw = glm::degrees(atan2(dir.z, dir.x));

    /*this->pitch = PITCH;
    this->yaw = YAW;*/

    this->fov = glm::radians(fov);
    focalDist = 0.1f;
    aperture = 0.0;
    this->width = width;
    this->height = height;
    this->aspectratio = (float)(float(this->width) / float(this->height));
    updateCamera();

}

Camera::Camera(const Camera& other)
{
    *this = other;
}

Camera& Camera::operator = (const Camera& other)
{
    ptrdiff_t l = (unsigned char*)&isMoving - (unsigned char*)&position.x;
    isMoving = memcmp(&position.x, &other.position.x, l) != 0;
    memcpy(&position.x, &other.position.x, l);
    return *this;
}

void Camera::offsetOrientation(float x, float y, bool constrainPitch)
{
    x *= MouseSensitivity;
    y *= MouseSensitivity;

    yaw += x;
    pitch += y;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    updateCamera();
}

void Camera::offsetPosition(glm::vec3 newPos)
{
    position += newPos;
    updateCamera();
}

void Camera::updateCamera()
{
    glm::vec3 forward_temp;
    forward_temp.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    forward_temp.y = sin(glm::radians(pitch));
    forward_temp.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    forward = glm::normalize(forward_temp);

    right = glm::normalize(glm::cross(forward, worldUp));
    up = glm::normalize(glm::cross(right, forward));
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        position += forward * velocity;
    if (direction == BACKWARD)
        position -= forward * velocity;
    if (direction == LEFT)
        position -= glm::normalize(glm::cross(forward, up)) * velocity;  //right * velocity;
    if (direction == RIGHT)
        position += glm::normalize(glm::cross(forward, up)) * velocity;  //right * velocity;
    updateCamera();
}