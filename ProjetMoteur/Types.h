#pragma once
#include <glm/glm.hpp>
#include <atomic>
#include <glm/gtc/matrix_transform.hpp>

struct Vertex
{
    glm::vec4 position;
    float r, g, b; 
    glm::vec2 uv;  
    glm::vec3 normal;
    glm::vec3 worldPos;
    float alpha = 1.0f;
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
    float alpha = 1.0f;
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
    float alpha = 1.0f;
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
    int trianglesSubmitted = 0;
    int trianglesFrustumCulled = 0;
    int trianglesProcessed = 0;

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
};

struct Box {
    int x0, y0, x1, y1; 
};

struct Particle {
    glm::vec3 position{ 0.0f };
    glm::vec3 velocity{ 0.0f };
    glm::vec4 color{ 1.0f };
    float size = 0.2f;
    float lifetime = 0.0f;
    float maxLifetime = 1.0f;
    bool active = false;
};

struct Transform
{
    glm::vec3 translation{ 0.0f, 0.0f, 0.0f };
    glm::vec3 rotation{ 0.0f, 0.0f, 0.0f }; 
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

    glm::mat4 GetModelMatrix() const
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, translation);
        model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        return model;
    }
};
