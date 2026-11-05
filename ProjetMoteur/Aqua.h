#pragma once
#pragma once

#include "Scene.h"
#include "ObjLoader.h"
#include "ParticleEmitter.h"
#include "Types.h"

class Aqua : public Scene
{
public:
    Aqua(SceneManager& sceneManager);
    ~Aqua() override = default;

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
    glm::mat4 m_lastModelMatrix = glm::mat4(1.0f);
    bool m_hasLastModelMatrix = false;

    T_OBJ m_objModel;
    bool m_loaded = false;
    float m_time = 0.0f;
    std::vector<Vertex> m_gpuVertices;

    GameObject m_fish;
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