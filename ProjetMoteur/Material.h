#pragma once

#include <glm/glm.hpp>
#include "Texture.h"

struct Material
{
    // Couleurs de la surface (RGB normalisés entre 0.0f et 1.0f)
    glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f); // Ka : Couleur/intensité dans l'ombre
    glm::vec3 diffuse = glm::vec3(0.8f, 0.8f, 0.8f); // Kd : Couleur de base sous une lumière blanche
    glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f); // Ks : Couleur du reflet brillant

    float shininess = 32.0f; 

    Texture texture;
    bool hasTexture = false;

    Material() = default;

    void Reset() {
        ambient = glm::vec3(0.2f);
        diffuse = glm::vec3(0.8f);
        specular = glm::vec3(0.5f);
        shininess = 32.0f;
        hasTexture = false;
    }
};