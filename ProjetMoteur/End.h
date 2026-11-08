#pragma once

#include "Scene.h"
#include "ObjLoader.h"
#include "ParticleEmitter.h"

class End : public Scene
{
public:
    End(SceneManager& sceneManager);
    ~End() override = default;

    void Update(float deltaTime) override;
    void Render(
        GraphicsPipeline& pipeline,
        SceneManager& sceneManager,
        const glm::mat4& viewMatrix,
        const glm::mat4& viewProjection,
        const glm::vec3& viewPos,
        float deltaTime
    ) override;
    void OnExit() override;


private:
    float m_hudAnimationTime = 0.0f;
    float m_waterTimer = 0.0f;
    ParticleEmitter m_magicEmitter;
};