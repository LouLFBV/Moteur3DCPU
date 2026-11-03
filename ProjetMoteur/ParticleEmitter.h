
#pragma once

#include <vector>
#include <cstddef>
#include <glm/glm.hpp>

#include "Types.h"
#include "GraphicsPipeline.h"

class ParticleEmitter
{
public:

    explicit ParticleEmitter(size_t maxParticles = 1000);

    void Update(float deltaTime);

    void Render(
        GraphicsPipeline& pipeline,
        const glm::mat4& viewMatrix,
        const glm::mat4& viewProjection
    );

    // -------------------------------------------------
    // CONTROLE DE L'EMISSION
    // -------------------------------------------------

    void SetEmissionEnabled(bool enabled);

    void Clear();

    size_t GetActiveCount() const;

    // -------------------------------------------------
    // EMISSIONS PONCTUELLES
    // -------------------------------------------------

    void Burst(int count);

    void BurstRadial(
        int count,
        float minSpeed,
        float maxSpeed
    );

    void BurstRing(
        int count,
        float minSpeed,
        float maxSpeed,
        float minVerticalSpeed = -0.2f,
        float maxVerticalSpeed = 0.5f
    );

    void BurstUpward(
        int count,
        float minSpeed,
        float maxSpeed
    );

    void Explode(
        const glm::vec3& origin,
        int count,
        float minSpeed,
        float maxSpeed
    );

    // -------------------------------------------------
    // CONFIGURATION DE L'EMETTEUR
    // -------------------------------------------------

    glm::vec3 position{ 0.0f };

    glm::vec3 velocityMin{ -0.5f, 1.5f, -0.5f };
    glm::vec3 velocityMax{ 0.5f, 3.0f, 0.5f };

    glm::vec3 gravity{ 0.0f, -0.5f, 0.0f };

    glm::vec4 colorStart{ 1.0f, 0.5f, 0.1f, 1.0f };
    glm::vec4 colorEnd{ 0.2f, 0.2f, 0.2f, 0.0f };

    float spawnRate = 60.0f;

    float lifeTimeMin = 0.8f;
    float lifeTimeMax = 1.5f;

    float sizeStart = 0.4f;
    float sizeEnd = 0.05f;


    // Rayon de dispersion à la naissance.
    float spawnRadius = 0.0f;

    // Freinage progressif des particules.
    float drag = 0.0f;

    // Force de rotation autour de l'émetteur.
    float swirlStrength = 0.0f;

    // Intensité des mouvements irréguliers.
    float turbulence = 0.0f;

    // Permet d'activer ou arrêter l'émission continue.
    bool emissionEnabled = true;

private:

    void EmitParticle();

    bool SpawnParticle(const glm::vec3& initialVelocity);

    glm::vec3 RandomDirection();

    float RandomFloat(float min, float max);

    std::vector<Particle> m_particles;

    float m_spawnAccumulator = 0.0f;
    float m_elapsedTime = 0.0f;
};
