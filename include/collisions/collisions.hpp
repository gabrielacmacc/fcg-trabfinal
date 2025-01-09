#pragma once

#include "../objects/objects.hpp"

glm::vec4 checkSphereToPlaneCollision(AABB a, Sphere b)
{
    glm::vec3 offset = {0.0f, 0.0f, 0.0f};

    float xPenetrationMin = a.min.x - (b.center.x - b.radius);
    float xPenetrationMax = (b.center.x + b.radius) - a.max.x;
    float yPenetrationMin = a.min.y - (b.center.y - b.radius);
    float yPenetrationMax = (b.center.y + b.radius) - a.max.y;
    float zPenetrationMin = a.min.z - (b.center.z - b.radius);
    float zPenetrationMax = (b.center.z + b.radius) - a.max.z;

    if (xPenetrationMin > 0.0f)
    {
        offset.x = -xPenetrationMin;
    }
    else if (xPenetrationMax > 0.0f)
    {
        offset.x = xPenetrationMax;
    }

    if (yPenetrationMin > 0.0f)
    {
        offset.y = -yPenetrationMin;
    }
    else if (yPenetrationMax > 0.0f)
    {
        offset.y = yPenetrationMax;
    }

    if (zPenetrationMin > 0.0f)
    {
        offset.z = -zPenetrationMin;
    }
    else if (zPenetrationMax > 0.0f)
    {
        offset.z = zPenetrationMax;
    }

    if (glm::length(offset) > 0.0f)
    {
        glm::vec3 normalizedDirection = glm::normalize(offset);
        return glm::vec4(round(normalizedDirection.x), round(normalizedDirection.y), round(normalizedDirection.z), 0.0f);
    }

    return glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

glm::vec3 AABBPointClosestToSphereCenter(AABB a, Sphere b)
{
    float x = std::max(a.min.x, std::min(b.center.x, a.max.x));
    float y = std::max(a.min.y, std::min(b.center.y, a.max.y));
    float z = std::max(a.min.z, std::min(b.center.z, a.max.z));
    return glm::vec3(x, y, z);
}

glm::vec4 checkSphereToAABBCollisionDirection(AABB a, Sphere b)
{
    glm::vec3 closestPoint = AABBPointClosestToSphereCenter(a, b);
    float dist = glm::distance(closestPoint, b.center);
    if (dist <= b.radius)
    {
        glm::vec4 collision_direction = glm::vec4(closestPoint.x, closestPoint.y, closestPoint.z, 1.0f) -
                                        glm::vec4(b.center.x, b.center.y, b.center.z, 1.0f);

        collision_direction = collision_direction / norm(collision_direction);
        return glm::vec4(round(collision_direction.x), round(collision_direction.y), round(collision_direction.z), 0.0f); // Vetor unitário da direção da colisão
    }
    return glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

// Se a distância entre os centros das esferas for menor do que a soma dos seus raios,
// é porque houve colisão entre elas
bool checkSphereToSphereCollision(Sphere a, Sphere b)
{
    float dist = glm::distance(a.center, b.center);
    if (dist < a.radius + b.radius)
    {
        return true;
    }
    return false;
}

glm::vec4 cancelCollisionMovement(glm::vec4 movement, std::vector<glm::vec4> collision_directions)
{
    for (auto &cd : collision_directions)
    {
        float result = dotproduct(cd, movement);
        if (result > 0)
        {
            if (cd.x != 0)
            {
                movement.x = 0;
            }
            else if (cd.y != 0)
            {
                movement.y = 0;
            }
            else if (cd.z != 0)
            {
                movement.z = 0;
            }
        }
    }
    return movement;
}
