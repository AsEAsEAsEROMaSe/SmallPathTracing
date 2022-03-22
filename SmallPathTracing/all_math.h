#pragma once

#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <glm/glm.hpp>

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double clamp(double x, double min, double max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

inline double random_double() {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max - min) * random_double();
}

inline int random_int(int min, int max) {
    // Returns a random integer in [min,max].
    return static_cast<int>(random_double(min, max + 1));
}

inline glm::vec3 random_in_unit_disk() {
    while (true) {
        auto p = glm::vec3(random_double(-1, 1), random_double(-1, 1), 0);
        if (glm::pow(glm::length(p),2) >= 1) continue;
        return p;
    }
}

//Vec3f CV(glm::vec3 vec)
//{
//    return { vec.x, vec.y, vec.z };
//}
//
//glm::vec3 CV(Vec3f vec)
//{
//    return { vec.x, vec.y, vec.z };
//}