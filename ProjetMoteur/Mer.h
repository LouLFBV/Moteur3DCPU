#pragma once

#include "Scene.h"
#include "ObjLoader.h"
#include "ParticleEmitter.h"

class Mer : public Scene
{
public:
    Mer(SceneManager& sceneManager);
    ~Mer() override = default;

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

    // Effets ponctuels.
    ParticleEmitter m_explosionEmitter{ 400 };
    ParticleEmitter m_sparkEmitter{ 250 };
    ParticleEmitter m_teleportEmitter{ 300 };
    ParticleEmitter m_waterEmitter{ 300 };

    // Effet magique continu.
    ParticleEmitter m_magicEmitter{ 400 };

    // Timers de démonstration.
    float m_explosionTimer = 0.0f;
    float m_teleportTimer = 0.0f;
    float m_waterTimer = 0.0f;

    bool m_particlesFrozen = false;
};