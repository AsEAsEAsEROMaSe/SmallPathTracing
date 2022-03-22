#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tuple>
#include "rtweekend.h"

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


class Camera
{
public:
    Camera(){}
    Camera(glm::vec3 pos, glm::vec3 lookAt, float fov, int width, int height);
    Camera(const Camera& other);
    Camera& operator = (const Camera& other);

    void offsetOrientation(float x, float y, bool constrainPitch = false);
    void offsetPosition(glm::vec3 val);
    void updateCamera();
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    glm::vec3 getDirection()
    {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        return direction;
    }
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(position, position + forward, up);
    }
    glm::vec3 position;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 forward;
    glm::vec3 worldUp;
    int width = 0;
    int height=0;
    float pitch, yaw, fov, focalDist, aperture;
    float aspectratio=0.0;
    bool isMoving;

    float MovementSpeed=90.0;
    float MouseSensitivity=0.05;
};


class raycamera {
public:
    raycamera() : raycamera(point3(0, 0, -1), point3(0, 0, 0), vec3(0, -1, 0), 40, 1, 0, 10) {}

    raycamera(
        point3 lookfrom,
        point3 lookat,
        vec3   vup,
        double vfov, // vertical field-of-view in degrees
        double aspect_ratio,
        double aperture,
        double focus_dist,
        double _time0 = 0,
        double _time1 = 0
    ) {
        auto theta = degrees_to_radians(vfov);
        auto h = tan(theta / 2);
        auto viewport_height = 2.0 * h;
        auto viewport_width = aspect_ratio * viewport_height;

        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        origin = lookfrom;
        horizontal = focus_dist * viewport_width * u;
        vertical = focus_dist * viewport_height * v;
        lower_left_corner = origin - horizontal / 2 - vertical / 2 - focus_dist * w;

        lens_radius = aperture / 2;
        time0 = _time0;
        time1 = _time1;
    }

    ray get_ray(double s, double t) const {
        //vec3 rd = lens_radius * random_in_unit_disk();
        vec3 offset = vec3(0,0,0);// u* rd.x() + v * rd.y();
        return ray(
            origin + offset,
            lower_left_corner + s * horizontal + t * vertical - origin - offset,
            random_double(time0, time1)
        );
    }

private:

    point3 origin;
    point3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    double lens_radius;
    double time0, time1;  // shutter open/close times
};