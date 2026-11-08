#pragma once
#include <glm/glm.hpp>
#include "Texture.h"


struct Material
{
    glm::vec3 ambient{ 0.2f };
    glm::vec3 diffuse{ 0.8f };
    glm::vec3 specular{ 0.5f };

    float shininess = 32.0f;

    Texture texture;
    bool hasTexture = false;


    void Reset()
    {
        ambient = glm::vec3(0.2f);
        diffuse = glm::vec3(0.8f);
        specular = glm::vec3(0.5f);

        shininess = 32.0f;

        hasTexture = false;
    }
};
