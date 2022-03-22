#pragma once
#include <cstdlib> 
#include <cstdio> 
#include <cmath> 
#include <fstream> 
#include <vector> 
#include <iostream> 
#include <cassert> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "Camera.h"
#include "hittable_list.h"
#include "material.h"
#include "pdf.h"
#include <omp.h>

color ray_color(
    const ray& r,
    const color& background,
    const hittable& world,
    shared_ptr<hittable> lights,
    int depth
) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0, 0, 0);

    // If the ray hits nothing, return the background color.
    if (!world.hit(r, 0.001, infinity, rec))
        return background;

    scatter_record srec;
    color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);

    if (!rec.mat_ptr->scatter(r, rec, srec))
        return emitted;

    if (srec.is_specular) {
        return srec.attenuation
            * ray_color(srec.specular_ray, background, world, lights, depth - 1);
    }

    auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
    mixture_pdf p(light_ptr, srec.pdf_ptr);
    ray scattered = ray(rec.p, p.generate(), r.time());
    auto pdf_val = p.value(scattered.direction());

    return emitted
        + srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered)
        * ray_color(scattered, background, world, lights, depth - 1)
        / pdf_val;
}


//
//#define MAX_RAY_DEPTH 5
//
//float mix(const float& a, const float& b, const float& mix)
//{
//    return b * mix + a * (1 - mix);
//}
//
//glm::vec3 trace(
//    const glm::vec3& rayorig,
//    const glm::vec3& raydir,
//    const std::vector<Sphere>& spheres,
//    const int& depth)
//{
//    //if (raydir.length() != 1) std::cerr << "Error " << raydir << std::endl;
//    float tnear = INFINITY;
//    const Sphere* sphere = NULL;
//    // find intersection of this ray with the sphere in the scene
//    for (unsigned i = 0; i < spheres.size(); ++i) 
//    {
//        float t0 = INFINITY, t1 = INFINITY;
//        if (spheres[i].intersect(rayorig, raydir, t0, t1)) 
//        {
//            if (t0 < 0) t0 = t1;
//            if (t0 < tnear) {
//                tnear = t0;
//                sphere = &spheres[i];
//            }
//        }
//    }
//    // if there's no intersection return black or background color
//    if (!sphere) return glm::vec3(0.0f);
//
//    glm::vec3 surfaceColor = glm::vec3(0.0f); // color of the ray/surfaceof the object intersected by the ray 
//    glm::vec3 phit = rayorig + raydir * tnear; // point of intersection 
//    glm::vec3 nhit = phit - sphere->center; // normal at the intersection point 
//    nhit = glm::normalize(nhit); // normalize normal direction 
//    // If the normal and the view direction are not opposite to each other
//    // reverse the normal direction. That also means we are inside the sphere so set
//    // the inside bool to true. Finally reverse the sign of IdotN which we want
//    // positive.
//    float bias = 1e-4; // add some bias to the point from which we will be tracing 
//    bool inside = false;
//    if (glm::dot(raydir, nhit) > 0) nhit = -nhit, inside = true;
//
//    if ((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH) 
//    {
//        float facingratio = -glm::dot(raydir, nhit);
//        // change the mix value to tweak the effect
//        float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1f);
//        // compute reflection direction (not need to normalize because all vectors
//        // are already normalized)
//        glm::vec3 refldir = raydir - nhit * 2.0f * glm::dot(raydir, nhit);
//        refldir = glm::normalize(refldir);
//        glm::vec3 reflection = trace(phit + nhit * bias, refldir, spheres, depth + 1);
//        glm::vec3 refraction = glm::vec3(0.0f);
//        // if the sphere is also transparent compute refraction ray (transmission)
//        if (sphere->transparency) 
//        {
//            float ior = 1.1f, eta = (inside) ? ior : 1.0f / ior; // are we inside or outside the surface? 
//            float cosi = -glm::dot(nhit, raydir);
//            float k = 1.0f - eta * eta * (1.0f - cosi * cosi);
//            glm::vec3 refrdir = raydir * eta + nhit * (eta * cosi - sqrt(k));
//            refrdir = glm::normalize(refrdir);
//            refraction = trace(phit - nhit * bias, refrdir, spheres, depth + 1);
//        }
//        // the result is a mix of reflection and refraction (if the sphere is transparent)
//        surfaceColor = (
//            reflection * fresneleffect +
//            refraction * (1.0f - fresneleffect) * sphere->transparency) * sphere->surfaceColor;
//    }
//    else 
//    {
//        // it's a diffuse object, no need to raytrace any further
//        for (unsigned i = 0; i < spheres.size(); ++i) 
//        {
//            if (spheres[i].emissionColor.x > 0) 
//            {
//                // this is a light
//                glm::vec3 transmission = glm::vec3(1.0f);
//                glm::vec3 lightDirection = spheres[i].center - phit;
//                lightDirection = glm::normalize(lightDirection);
//                for (unsigned j = 0; j < spheres.size(); ++j) 
//                {
//                    if (i != j) 
//                    {
//                        float t0, t1;
//                        if (spheres[j].intersect(phit + nhit * bias, lightDirection, t0, t1)) 
//                        {
//                            transmission = glm::vec3(0.0f);
//                            break;
//                        }
//                    }
//                }
//                surfaceColor += sphere->surfaceColor * transmission *
//                    std::max(float(0), glm::dot(nhit, lightDirection)) * spheres[i].emissionColor;
//            }
//        }
//    }
//
//    return surfaceColor + sphere->emissionColor;
//}

bool na_dva = true;
bool one_raz = false;
int value_count = 2;
bool this_yes(int j, int i)
{
    if (one_raz == false)
    {
        return true;
    }
    else
    {
        if (na_dva)
        {
            if (i % value_count == 0 && j % value_count == 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (i % value_count == 0 && j % value_count == 0)
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    }
}
color background(0, 0, 0);

auto time0 = 0.0;
auto time1 = 1.0;
const int max_depth = 10;
auto dist_to_focus = 10.0;
auto aperture = 0.0;
const int samples_per_pixel = 5;

glm::vec3 trn(vec3 v)
{
    return glm::vec3(static_cast<float>(v.x()), static_cast<float>(v.y()), static_cast<float>(v.z()));
}

vec3 trn(glm::vec3 v)
{
    return vec3(static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z));
}

void render(std::vector<sf::Vector2i>& poll_index, Camera& camera, hittable_list& world, std::shared_ptr<hittable_list> lights, sf::VertexArray& ver_arr,  int width, int height)
{
    /*if (one_raz == false)
    {
        one_raz = true;
    }
    if (na_dva == false)
    {
        na_dva = true;
    }
    else
    {
        na_dva = false;
    }*/

    raycamera cam(trn(camera.position), trn(camera.getDirection() + camera.position), trn(camera.up), camera.fov, camera.aspectratio, aperture, dist_to_focus, time0, time1);

    for (int idx = 0; idx < poll_index.size(); idx++)
    {
        int j = poll_index[idx].y;
        int i = poll_index[idx].x;

        //if (this_yes(j, i))
        {
            auto u = (i + 0.1) / (width - 1);//x
            auto v = (j + 0.1) / (height - 1);//y
            auto ray_m = cam.get_ray(u, v);

            color pixel_color(0, 0, 0);
            for (unsigned ss = 0; ss < samples_per_pixel; ss++)
            {
                pixel_color += ray_color(ray_m, background, world, lights, max_depth);
            }

            float r = pixel_color.x();
            float g = pixel_color.y();
            float b = pixel_color.z();

            // Replace NaN components with zero. See explanation in Ray Tracing: The Rest of Your Life.
            if (r != r) r = 0.0;
            if (g != g) g = 0.0;
            if (b != b) b = 0.0;

            // Divide the color by the number of samples and gamma-correct for gamma=2.0.
            float scale = 1.0 / samples_per_pixel;
            r = sqrt(scale * r);
            g = sqrt(scale * g);
            b = sqrt(scale * b);

            ver_arr[i + j * width].position.x = i;
            ver_arr[i + j * width].position.y = j;
            ver_arr[i + j * width].color.r = static_cast<int>(256 * clamp(r, 0.0, 0.999));
            ver_arr[i + j * width].color.g = static_cast<int>(256 * clamp(g, 0.0, 0.999));
            ver_arr[i + j * width].color.b = static_cast<int>(256 * clamp(b, 0.0, 0.999));
            ver_arr[i + j * width].color.a = 255;
        }
    }
}