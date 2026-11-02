#pragma once

#include <memory>
#include <vector>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "Camera.h"
#include "Types.h"

class GraphicsPipeline;
class SceneManager;


class Scene
{
public:
    Scene(SceneManager& sceneManager);
    virtual ~Scene() = default;

    virtual void OnEnter() ;
    virtual void Update(float deltaTime) = 0;
    virtual void Render(GraphicsPipeline& pipeline, SceneManager& sceneManager, const glm::mat4& viewProjection, const glm::vec3& viewPos, float deltaTime) = 0;
    virtual void OnExit() = 0;

    void GenerateTerrain(int size, float cellSize);
    Camera& GetCamera();

protected:
    Camera m_sceneCamera{ glm::vec3(0.0f, 0.0f, 0.0f) };
    std::vector<Vertex> m_terrainVertices;
    int m_gridSize = 80;
    float m_cellSize = 1.0f;
};
