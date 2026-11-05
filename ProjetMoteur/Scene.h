#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "Camera.h"
#include "Types.h"
#include "Texture.h"

class GraphicsPipeline;
class SceneManager;


struct Key
{
    float t;
    float v;
    bool smoothing = false;
};

class Scene
{
public:
    Scene(SceneManager& sceneManager);
    virtual ~Scene() = default;

    virtual void Update(float deltaTime);
    virtual void Render(
        GraphicsPipeline& pipeline,
        SceneManager& sceneManager,
        const glm::mat4& viewMatrix,
        const glm::mat4& viewProjection,
        const glm::vec3& viewPos,
        float time
    ) = 0;
    virtual void OnExit() = 0;

    Camera& GetCamera();
    void GenerateTerrain(int size, float cellSize);

    void AddKeyframe(float* target, float t, float v, bool smoothing = false);
    float GetSceneTime() const { return m_sceneTime; }

protected:
    std::unordered_map<float*, std::vector<Key>> keys;
    Camera m_sceneCamera{ glm::vec3(0.0f, 0.0f, 0.0f) };
    float m_sceneTime = 0.0f;
    std::vector<Vertex> m_terrainVertices;
    int m_gridSize = 80;
    float m_cellSize = 1.0f;
    Texture m_fontTexture;

    std::vector<GameObject> m_gameObjects;
};
