#pragma once

#include <memory>
#include <vector>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "Camera.h"

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

    Camera& GetCamera();

protected:
    Camera m_sceneCamera{ glm::vec3(0.0f, 0.0f, 0.0f) };
};
