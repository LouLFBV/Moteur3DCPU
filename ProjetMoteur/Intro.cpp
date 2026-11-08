#include "Intro.h"
#include "GraphicsPipeline.h"
#include "Camera.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "SceneManager.h"

Intro::Intro(SceneManager& sceneManager) : Scene(sceneManager)
{
    if (m_fontTexture.LoadFromFile("external/Assets/boxy_bold_font_5.png"))
        std::cout << "font texte charge !\n";
    else
        std::cout << "font texte pas trouve !\n";
}




void Intro::Update(float deltaTime)
{

    Scene::Update(deltaTime);
    // Temps global de la scène.
    m_time += deltaTime;

    // Temps de l'animation du HUD.
    // L'animation est gelée en mode Edit.
    if (!m_particlesFrozen)
    {
        m_hudAnimationTime += deltaTime;
    }


}



void Intro::Render(GraphicsPipeline& pipeline, SceneManager& sceneManager, const glm::mat4& viewMatrix, const glm::mat4& viewProjection, const glm::vec3& viewPos, float deltaTime)
{


    // =====================================================
    // ANIMATION DU TEXTE
    // =====================================================

// Durées des différentes phases.
    constexpr float fadeInDuration = 0.8f;
    constexpr float holdDuration = 1.4f;
    constexpr float fadeOutDuration = 0.8f;
    constexpr float pauseDuration = 1.0f;

    constexpr float cycleDuration =
        fadeInDuration + holdDuration +
        fadeOutDuration + pauseDuration;

    // Temps dans le cycle actuel.
    float animationTime = std::fmod(
        m_hudAnimationTime,
        cycleDuration
    );

    float alpha = 0.0f;
    float textX = 20.0f;

    // Fonction d'interpolation douce.
    auto SmoothStep = [](float t)
        {
            t = std::clamp(t, 0.0f, 1.0f);
            return t * t * (3.0f - 2.0f * t);
        };

    // PHASE 1 : APPARITION
    if (animationTime < fadeInDuration)
    {
        float progress = animationTime / fadeInDuration;
        float eased = SmoothStep(progress);

        alpha = eased;

        // Le texte arrive depuis la gauche.
        textX = -60.0f + 80.0f * eased;
    }

    // PHASE 2 : AFFICHAGE
    else if (animationTime < fadeInDuration + holdDuration)
    {
        alpha = 1.0f;
        textX = 20.0f;
    }

    // PHASE 3 : DISPARITION
    else if (animationTime <
        fadeInDuration + holdDuration + fadeOutDuration)
    {
        float progress =
            (animationTime - fadeInDuration - holdDuration)
            / fadeOutDuration;

        float eased = SmoothStep(progress);

        alpha = 1.0f - eased;

        // Le texte repart vers la droite.
        textX = 20.0f + 80.0f * eased;
    }

    // PHASE 4 : INVISIBLE
    else
    {
        alpha = 0.0f;
    }

    // Conversion de l'alpha en entier [0, 255].
    uint32_t alphaValue = static_cast<uint32_t>(
        std::round(alpha * 255.0f)
        );

    // Couleur Cyan avec transparence variable.
    uint32_t textColor =
        (alphaValue << 24) | 0x0000FFFF;

    // Affichage du texte animé.
    pipeline.DrawText2D(
        static_cast<int>(textX),
        70,
        "World Adventure",
        m_fontTexture,
        textColor
    );


    pipeline.RenderScene();


}

void Intro::OnExit()
{
    std::cout << "Le Village Scene quittee\n";
}

void Intro::SetParticlesFrozen(bool frozen)
{
    m_particlesFrozen = frozen;
}

void Intro::SetEditMode(bool editMode)
{
    m_isEditMode = editMode;
    SetParticlesFrozen(editMode);
}