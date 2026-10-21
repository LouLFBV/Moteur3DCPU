#pragma once
#include <glm/glm.hpp>

struct Vertex
{
    glm::vec4 position;
    float r, g, b;
};

struct ScreenVertex
{
    int x, y; // pixel
    float z;  // profondeur NDC
    float r, g, b; // couleur
    float invW; // Ajouté pour la perspective !
};

struct BordLigne
{
    int x;
    float z;
    float r, g, b;
    float invW; // Ajouté pour la perspective !
};