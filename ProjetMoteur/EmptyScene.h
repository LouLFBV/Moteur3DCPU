#pragma once

#include "Scene.h"

class EmptyScene : public Scene
{
public:
    EmptyScene(SceneManager& sceneManager);
    ~EmptyScene() override = default;

    void OnEnter() override;
    void Update(float deltaTime) override;
    void Render(GraphicsPipeline& pipeline, SceneManager& sceneManager, const glm::mat4& viewProjection, const glm::vec3& viewPos, float deltaTime) override;
    void OnExit() override;

private:
    float m_time = 0.0f;
};