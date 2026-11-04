#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "Camera.h"

class GraphicsPipeline;
class SceneManager;


struct Key
{
    float t;
    float v;
};

class Scene
{
public:
    Scene(SceneManager& sceneManager);
    virtual ~Scene() = default;

    virtual void OnEnter() ;
    virtual void Update(float deltaTime);
    virtual void Render(GraphicsPipeline& pipeline, SceneManager& sceneManager, const glm::mat4& viewProjection, const glm::vec3& viewPos, float deltaTime) = 0;
    virtual void OnExit() = 0;

    Camera& GetCamera();

    void AddKeyframe(float* target, float t, float v);
    float GetSceneTime() const { return m_sceneTime; }

protected:
    std::unordered_map<float*, std::vector<Key>> keys;
    Camera m_sceneCamera{ glm::vec3(0.0f, 0.0f, 0.0f) };
    float m_sceneTime = 0.0f;
};
