#pragma once
#include <glm/glm.hpp>

struct Vertex
{
    glm::vec4 position;
    float r, g, b; 
    glm::vec2 uv;  
    glm::vec3 normal;
};

struct ScreenVertex
{
    int x, y; 
    float z;  
    float r, g, b; 
    float invW; 
    float u, v;
    float nx, ny, nz;
};

struct BordLigne
{
    int x;
    float z;
    float r, g, b;
    float invW;
    float u, v;
    float nx, ny, nz;
};

struct Cube 
{
    Vertex faceDown[4];
    Vertex faceTop[4];
    Vertex faceFront[4];
    Vertex faceBack[4];
    Vertex faceLeft[4];
    Vertex faceRight[4];
};