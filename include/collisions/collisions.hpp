#pragma once

#include "../objects/objects.hpp"

bool checkAABBCollision(AABB a, AABB b)
{
    if (a.min.x <= b.max.x && a.max.x >= b.min.x &&
        a.min.y <= b.max.y && a.max.y >= b.min.y &&
        a.min.z <= b.max.z && a.max.z >= b.min.z)
    {
        return true;
    };

    return false;
}

glm::vec4 checkAABBToPlaneCollision(AABB a, AABB b) {
    glm::vec4 offset = {0.0f, 0.0f, 0.0f, 0.0f};

    float xPenetrationMin = b.min.x - a.min.x;
    float xPenetrationMax = b.max.x - a.max.x;
    float yPenetrationMin = b.min.y - a.min.y;
    float yPenetrationMax = b.max.y - a.max.y;
    float zPenetrationMin = b.min.z - a.min.z;
    float zPenetrationMax = b.max.z - a.max.z;

    if (xPenetrationMin > 0.0f) {
        offset.x = xPenetrationMin;   
    } else if (xPenetrationMax < 0.0f) {
        offset.x = xPenetrationMax;
    }

    if (yPenetrationMin > 0.0f) {
        offset.y = yPenetrationMin;
    } else if (yPenetrationMax < 0.0f) {
        offset.y = yPenetrationMax;
    }

    if (zPenetrationMin > 0.0f) {
        offset.z = zPenetrationMin;
    } else if (zPenetrationMax < 0.0f) {
        offset.z = zPenetrationMax;
    }

    return offset;
}

glm::vec4 checkSphereToPlaneCollision(AABB a, Sphere b) {
    glm::vec4 offset = {0.0f, 0.0f, 0.0f, 0.0f};

    float xPenetrationMin = a.min.x - (b.center.x - b.radius);
    float xPenetrationMax = (b.center.x + b.radius) - a.max.x;
    float yPenetrationMin = a.min.y - (b.center.y - b.radius);
    float yPenetrationMax = (b.center.y + b.radius) - a.max.y;
    float zPenetrationMin = a.min.z - (b.center.z - b.radius);
    float zPenetrationMax = (b.center.z + b.radius) - a.max.z;

    if (xPenetrationMin > 0.0f) {
        offset.x = xPenetrationMin;
    } else if (xPenetrationMax > 0.0f) {
        offset.x = -xPenetrationMax;
    }

    if (yPenetrationMin > 0.0f) {
        offset.y = yPenetrationMin;
    } else if (yPenetrationMax > 0.0f) {
        offset.y = -yPenetrationMax;
    }

    if (zPenetrationMin > 0.0f) {
        offset.z = zPenetrationMin;
    } else if (zPenetrationMax > 0.0f) {
        offset.z = -zPenetrationMax;
    }

    return offset;
}

glm::vec4 checkSphereToSphereCollision(Sphere a, Sphere b)
{
    glm::vec4 offset = {0.0f, 0.0f, 0.0f, 0.0f};
    return offset;
}
