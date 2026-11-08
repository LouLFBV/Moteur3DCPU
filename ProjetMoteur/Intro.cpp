#include "Intro.h"
#include "GraphicsPipeline.h"
#include "SceneManager.h"
#include <algorithm>
#include <cmath>
#include <iostream>


Intro::Intro(SceneManager& sceneManager)
    : Scene(sceneManager)
{
    if (m_fontTexture.LoadFromFile("external/Assets/boxy_bold_font_5.png")) {
        std::cout << "[Intro] Police chargee !\n";
    }
    else {
        std::cerr << "[Intro] Impossible de charger la police !\n";
    }
}


void Intro::Update(float deltaTime)
{
    Scene::Update(deltaTime);

    if (!m_particlesFrozen) {
        m_hudAnimationTime += deltaTime;
    }
}


void Intro::Render(
    GraphicsPipeline& pipeline,
    SceneManager& sceneManager,
    const glm::mat4& viewMatrix,
    const glm::mat4& viewProjection,
    const glm::vec3& viewPos,
    float time
)
{
    constexpr float fadeInDuration = 0.8f;
    constexpr float holdDuration = 1.4f;
    constexpr float fadeOutDuration = 0.8f;
    constexpr float pauseDuration = 1.0f;

    constexpr float cycleDuration =
        fadeInDuration +
        holdDuration +
        fadeOutDuration +
        pauseDuration;

    const float animationTime = std::fmod(
        m_hudAnimationTime,
        cycleDuration
    );

    float alpha = 0.0f;
    float textX = 20.0f;

    const auto smoothStep = [](float value)
        {
            value = std::clamp(value, 0.0f, 1.0f);

            return value * value * (3.0f - 2.0f * value);
        };

    if (animationTime < fadeInDuration){
        const float progress = animationTime / fadeInDuration;
        const float easedProgress = smoothStep(progress);

        alpha = easedProgress;

        textX = -60.0f + 80.0f * easedProgress;
    }

    else if (animationTime < fadeInDuration + holdDuration) {
        alpha = 1.0f;
        textX = 20.0f;
    }

    else if (animationTime < fadeInDuration + holdDuration + fadeOutDuration) {
        const float progress =
            (animationTime - fadeInDuration - holdDuration)
            / fadeOutDuration;

        const float easedProgress = smoothStep(progress);

        alpha = 1.0f - easedProgress;

        textX = 20.0f + 80.0f * easedProgress;
    }

    else {
        alpha = 0.0f;
    }

    const uint32_t alphaValue = static_cast<uint32_t>( std::round(alpha * 255.0f));

    const uint32_t textColor = (alphaValue << 24) | 0x0000FFFF;

    pipeline.DrawText2D(
        static_cast<int>(textX),
        70,
        "World Adventure",
        m_fontTexture,
        textColor
    );

    pipeline.RenderScene();
}
