#pragma once

#include "Scene.h"
#include "ObjLoader.h"
#include "ParticleEmitter.h"

class LastScene : public Scene
{
public:
    LastScene(SceneManager& sceneManager);
    ~LastScene() override = default;

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

    void SetParticlesFrozen(bool frozen);

    void SetEditMode(bool editMode);
    bool m_isEditMode = false;
private:
    float m_time = 0.0f;
    // Animation du texte
    float m_hudAnimationTime = 0.0f;

    // Feu et fumée.
    ParticleEmitter m_fireEmitter{ 350 };
    ParticleEmitter m_smokeEmitter{ 400 };


    bool m_particlesFrozen = false;
    bool m_textPrinted = false;
};