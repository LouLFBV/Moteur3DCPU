
#include "ParticleEmitter.h"
#include "GraphicsPipeline.h"

#include <random>
#include <algorithm>
#include <cmath>

ParticleEmitter::ParticleEmitter(size_t maxParticles)
{
    m_particles.resize(maxParticles);
}

float ParticleEmitter::RandomFloat(float min, float max)
{
    if (min > max) {
        std::swap(min, max);
    }

    static std::mt19937 gen(std::random_device{}());

    std::uniform_real_distribution<float> distribution(min, max);

    return distribution(gen);
}
glm::vec3 ParticleEmitter::RandomDirection()
{
    glm::vec3 direction;

    do {
        direction = glm::vec3(
            RandomFloat(-1.0f, 1.0f),
            RandomFloat(-1.0f, 1.0f),
            RandomFloat(-1.0f, 1.0f)
        );
    } while (glm::dot(direction, direction) < 0.0001f);

    return glm::normalize(direction);
}

bool ParticleEmitter::SpawnParticle(const glm::vec3& initialVelocity)
{
    for (auto& p : m_particles) {
        if (!p.active) {
            p.active = true;

            p.position = position + RandomDirection() * RandomFloat(0.0f, spawnRadius);

            p.velocity = initialVelocity;

            p.maxLifetime = RandomFloat(lifeTimeMin, lifeTimeMax);

            p.lifetime = p.maxLifetime;

            p.size = sizeStart;
            p.color = colorStart;

            return true;
        }
    }

    return false;
}

void ParticleEmitter::EmitParticle()
{
    glm::vec3 velocity(
        RandomFloat(velocityMin.x, velocityMax.x),
        RandomFloat(velocityMin.y, velocityMax.y),
        RandomFloat(velocityMin.z, velocityMax.z)
    );

    SpawnParticle(velocity);
}

void ParticleEmitter::SetEmissionEnabled(bool enabled)
{
    emissionEnabled = enabled;

    if (!enabled) {
        m_spawnAccumulator = 0.0f;
    }
}

void ParticleEmitter::Clear()
{
    for (auto& p : m_particles) {
        p.active = false;
    }

    m_spawnAccumulator = 0.0f;
    m_elapsedTime = 0.0f;
}

size_t ParticleEmitter::GetActiveCount() const
{
    size_t count = 0;

    for (const auto& p : m_particles) {
        if (p.active) {
            count++;
        }
    }

    return count;
}

void ParticleEmitter::Burst(int count)
{
    for (int i = 0; i < count; ++i) {
        EmitParticle();
    }
}

void ParticleEmitter::BurstRadial(int count, float minSpeed, float maxSpeed) {
    for (int i = 0; i < count; ++i) {
        glm::vec3 direction = RandomDirection();

        float speed = RandomFloat(minSpeed, maxSpeed);

        SpawnParticle(direction * speed);
    }
}

void ParticleEmitter::BurstRing(int count, float minSpeed, float maxSpeed, float minVerticalSpeed, float maxVerticalSpeed)
{
    constexpr float PI = 3.14159265359f;

    for (int i = 0; i < count; i++)  {
        float angle = RandomFloat(0.0f, 2.0f * PI);

        float verticalSpeed = RandomFloat(minVerticalSpeed, maxVerticalSpeed);

        glm::vec3 direction(
            std::cos(angle),
            verticalSpeed,
            std::sin(angle)
        );

        direction = glm::normalize(direction);

        float speed = RandomFloat(minSpeed, maxSpeed);

        SpawnParticle(direction * speed);
    }
}

void ParticleEmitter::BurstUpward(int count, float minSpeed, float maxSpeed)
{
    for (int i = 0; i < count; ++i) {
        glm::vec3 direction(
            RandomFloat(-1.0f, 1.0f),
            RandomFloat(0.15f, 1.0f),
            RandomFloat(-1.0f, 1.0f)
        );

        direction = glm::normalize(direction);

        float speed = RandomFloat(minSpeed, maxSpeed);

        SpawnParticle(direction * speed);
    }
}

void ParticleEmitter::Explode(const glm::vec3& origin, int count, float minSpeed, float maxSpeed)
{
    position = origin;

    BurstRadial(count, minSpeed, maxSpeed);
}

void ParticleEmitter::Update(float deltaTime)
{
    if (deltaTime <= 0.0f) {
        return;
    }

    m_elapsedTime += deltaTime;
    
    for (auto& p : m_particles) {
        if (!p.active) {
            continue;
        }

        p.lifetime -= deltaTime;

        if (p.lifetime <= 0.0f) {
            p.active = false;
            continue;
        }

        float t = 1.0f - (p.lifetime / p.maxLifetime);

        p.velocity += gravity * deltaTime;

        if (swirlStrength > 0.0f) {
            glm::vec3 offset = p.position - position;

            glm::vec3 tangent(
                -offset.z,
                0.0f,
                offset.x
            );

            float lengthSquared = glm::dot(tangent, tangent);

            if (lengthSquared > 0.0001f) {
                tangent = glm::normalize(tangent);

                p.velocity += tangent * swirlStrength * deltaTime;
            }
        }

        if (turbulence > 0.0f) {
            float phase = m_elapsedTime * 2.0f + p.position.y;

            p.velocity.x += std::sin(phase + p.position.z) * turbulence * deltaTime;

            p.velocity.z += std::cos(phase + p.position.x) * turbulence * deltaTime;
        }

        if (drag > 0.0f) {
            p.velocity *= std::exp(-drag * deltaTime);
        }

        p.position += p.velocity * deltaTime;

        p.size = glm::mix(sizeStart, sizeEnd, t);

        p.color = glm::mix(colorStart, colorEnd, t);
    }

    if (emissionEnabled && spawnRate > 0.0f) {
        m_spawnAccumulator += deltaTime * spawnRate;

        while (m_spawnAccumulator >= 1.0f) {
            EmitParticle();
            m_spawnAccumulator -= 1.0f;
        }
    }
}

void ParticleEmitter::Render(GraphicsPipeline& pipeline, const glm::mat4& viewMatrix, const glm::mat4& viewProjection)
{
    glm::vec3 cameraRight = glm::normalize(glm::vec3(
        viewMatrix[0][0],
        viewMatrix[1][0],
        viewMatrix[2][0]
    ));

    glm::vec3 cameraUp = glm::normalize(glm::vec3(
        viewMatrix[0][1],
        viewMatrix[1][1],
        viewMatrix[2][1]
    ));

    // Préparation des particules visibles.
    struct SortedParticle
    {
        const Particle* particle;
        float depth;
    };

    std::vector<SortedParticle> sortedParticles;

    sortedParticles.reserve(m_particles.size());

    for (const auto& p : m_particles) {
        if (!p.active) {
            continue;
        }

        glm::vec4 cameraPosition = viewMatrix * glm::vec4(p.position, 1.0f);

        sortedParticles.push_back({
            &p,
            -cameraPosition.z
            });
    }

    // Transparence : dessiner les particules éloignées en premier.
    std::stable_sort(
        sortedParticles.begin(),
        sortedParticles.end(),
        [](const SortedParticle& a, const SortedParticle& b) {
            return a.depth > b.depth;
        }
    );

    for (const auto& sorted : sortedParticles) {
        const Particle& p = *sorted.particle;

        float halfSize = p.size * 0.5f;

        glm::vec3 posTL = p.position - cameraRight * halfSize + cameraUp * halfSize;

        glm::vec3 posTR = p.position + cameraRight * halfSize + cameraUp * halfSize;

        glm::vec3 posBL = p.position - cameraRight * halfSize - cameraUp * halfSize;

        glm::vec3 posBR = p.position + cameraRight * halfSize - cameraUp * halfSize;

        Vertex vTL{};
        Vertex vTR{};
        Vertex vBL{};
        Vertex vBR{};

        vTL.position = glm::vec4(posTL, 1.0f);
        vTR.position = glm::vec4(posTR, 1.0f);
        vBL.position = glm::vec4(posBL, 1.0f);
        vBR.position = glm::vec4(posBR, 1.0f);

        auto SetParticleColor = [&p](Vertex& v) {
                v.r = p.color.r;
                v.g = p.color.g;
                v.b = p.color.b;
                v.alpha = p.color.a;

                v.normal = glm::vec3(0.0f, 0.0f, 1.0f);
                v.worldPos = p.position;
            };

        SetParticleColor(vTL);
        SetParticleColor(vTR);
        SetParticleColor(vBL);
        SetParticleColor(vBR);

        vTL.uv = { 0.0f, 0.0f };
        vTR.uv = { 1.0f, 0.0f };
        vBL.uv = { 0.0f, 1.0f };
        vBR.uv = { 1.0f, 1.0f };

        pipeline.DrawSquare(
            vBL,
            vBR,
            vTR,
            vTL,
            viewProjection
        );
    }
}
