#pragma once

#include "Scene.h"

class Intro : public Scene
{
public:
    explicit Intro(SceneManager& sceneManager);
    ~Intro() override = default;

    void Update(float deltaTime) override;

    void Render(
        GraphicsPipeline& pipeline,
        SceneManager& sceneManager,
        const glm::mat4& viewMatrix,
        const glm::mat4& viewProjection,
        const glm::vec3& viewPos,
        float time
    ) override;
};