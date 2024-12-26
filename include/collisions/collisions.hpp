#pragma one

#include "../objects/objects.hpp"

bool checkAABBCollision(AABB a, AABB b) {
    if (a.min.x <= b.max.x && a.max.x >= b.min.x && 
        a.min.y <= b.max.y && a.max.y >= b.min.y && 
        a.min.z <= b.max.z && a.max.z >= b.min.z) {
            return true;
    };

    return false;
}

glm::vec4 checkPlaneCollision(AABB a, AABB b) {
    glm::vec4 offset = {0.0f, 0.0f, 0.0f, 0.0f};

    if (a.min.x < b.min.x) {
        offset.x = b.min.x - a.min.x;   
    }
    if (a.max.x > b.max.x) {
        offset.x = b.max.x - a.max.x;
    }
    if (a.max.z > b.max.z) {
        offset.z = b.max.z - a.max.z;
    }
    if (a.min.z < b.min.z) {
        offset.z = b.min.z - a.min.z;
    }
    if (a.min.y < b.min.y) {
        offset.y = b.min.y - a.min.y;
    }
    if (a.max.y > b.max.y) { //rever
        offset.y = b.max.y - a.max.y;
    }

    return offset;
}

glm::vec4 checkSphereToSphereCollision(Sphere a, Sphere b) {
    glm::vec4 offset = {0.0f, 0.0f, 0.0f, 0.0f};
    return offset;
}
