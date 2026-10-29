#pragma once
#include <glm/glm.hpp>
#include <atomic>

struct Vertex
{
    glm::vec4 position;
    float r, g, b; 
    glm::vec2 uv;  
    glm::vec3 normal;
    glm::vec3 worldPos;
};

struct ScreenVertex
{
    int x, y; 
    float z;  
    float r, g, b; 
    float invW; 
    float u, v;
    float nx, ny, nz;
    glm::vec3 worldPos;
};

struct BordLigne
{
    int x;
    float z;
    float r, g, b;
    float invW;
    float u, v;
    float nx, ny, nz;
    glm::vec3 worldPos;
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


struct RenderStats {
    int trianglesProcessed = 0;
    int trianglesRasterized = 0;
    std::atomic<int> pixelsProcessed{ 0 };
    int threadsUsed = 1;
    float frameTimeMs = 0.0f;
};

enum class ClipPlane {
    Near,
    Far,
    Left,
    Right,
    Top,
    Bottom
};

struct T_OBJVertex
{
    glm::vec4 position;
    float nx, ny, nz;
    float tx, ty;
    float r, g, b;
};

struct DrawCommand {
    Vertex v0, v1, v2;
    glm::mat4 mvp;
};