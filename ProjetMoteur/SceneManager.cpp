#include "SceneManager.h"
#include "GraphicsPipeline.h"
#include "Scene.h"
#include <algorithm>
#include <iostream>

SceneManager::SceneManager()
{
    SetFadeDuration(1.0f);
}

void SceneManager::AddScene(std::unique_ptr<Scene> scene, float t_start, float t_end)
{
    m_listScene.push_back({ std::move(scene), t_start, t_end });

    if (m_currentIndex == -1) {
        SetScene(0);
    }
}

void SceneManager::NextScene(bool isEditMode )
{
    std::cout << "NextScene\n";
    if (m_listScene.empty()) return;

    // Calcul de l'index suivant avec boucle automatique grâce au modulo %
    size_t nextIndex = m_currentIndex + 1;

    if (nextIndex >= m_listScene.size())
    {
        m_isRunning = false;
        return;
    }

    SetScene(nextIndex, isEditMode);
}

void SceneManager::SetScene(size_t index, bool isEditMode)
{
    std::cout << "SetScene\n";
    if (index >= m_listScene.size()) return;

    // Première scène : activation immédiate.
    if (m_currentIndex == -1 || isEditMode)
    {
        ActivateScene(index);

        m_fadeAlpha = 1.0f;
        m_fadeTimer = 0.0f;
        m_fadeState = FadeState::FadeIn;

        return;
    }

    // Évite de relancer une transition vers la scène actuelle.
    if (static_cast<int>(index) == m_currentIndex) return;

    // On mémorise la scène à charger.
    m_pendingSceneIndex = static_cast<int>(index);

    // On commence le Fade Out.
    m_fadeTimer = 0.0f;
    m_fadeAlpha = 0.0f;
    m_fadeState = FadeState::FadeOut;
}

void SceneManager::ActivateScene(size_t index)
{
    if (index >= m_listScene.size()) return;

    // Quitter l'ancienne scène.
    if (m_currentIndex >= 0 &&
        m_currentIndex < static_cast<int>(m_listScene.size()))
    {
        m_listScene[m_currentIndex].scene->OnExit();
    }

    m_currentIndex = static_cast<int>(index);

}

void SceneManager::UpdateFade(float deltaTime)
{
    if (m_fadeState == FadeState::None)
        return;

    m_fadeTimer += deltaTime;

    float progress = std::clamp(
        m_fadeTimer / m_fadeDuration,
        0.0f,
        1.0f
    );

    // Interpolation douce.
    float eased = progress * progress * (3.0f - 2.0f * progress);

    if (m_fadeState == FadeState::FadeOut)
    {
        m_fadeAlpha = eased;

        // Écran complètement opaque.
        if (progress >= 1.0f)
        {
            if (m_pendingSceneIndex >= 0)
            {
                ActivateScene(static_cast<size_t>(m_pendingSceneIndex));
                m_pendingSceneIndex = -1;
            }

            m_fadeTimer = 0.0f;
            m_fadeState = FadeState::FadeIn;
        }
    }
    else if (m_fadeState == FadeState::FadeIn)
    {
        m_fadeAlpha = 1.0f - eased;

        if (progress >= 1.0f)
        {
            m_fadeAlpha = 0.0f;
            m_fadeState = FadeState::None;
        }
    }
}

void SceneManager::RenderFade(GraphicsPipeline& pipeline) const
{
    if (m_fadeAlpha <= 0.0f)
        return;

    // Application directe du fondu sur l'image calculée
    pipeline.DrawFadeOverlay(m_fadeAlpha, m_fadeColor);
}

void SceneManager::Update(float deltaTime, bool isEditMode)
{
    float effectiveDeltaTime = isEditMode ? 0.0f : deltaTime;

    if (!isEditMode)
    {
        m_globalTime += deltaTime;
    }

    // Animation du fondu.
    UpdateFade(deltaTime);

    if (m_currentIndex >= 0 &&
        m_currentIndex < static_cast<int>(m_listScene.size()))
    {
        clip& currentClip = m_listScene[m_currentIndex];

        currentClip.scene->Update(effectiveDeltaTime);

        // Pas de nouveau changement de scène pendant une transition.
        if (!isEditMode && m_fadeState == FadeState::None)
        {
            float duration = currentClip.t_end - currentClip.t_start;

            if (duration > 0.0f &&
                currentClip.scene->GetSceneTime() >= duration)
            {
                NextScene(isEditMode);
            }
        }
    }
}

void SceneManager::Render(
    GraphicsPipeline& pipeline,
    const glm::mat4& viewMatrix,
    const glm::mat4& viewProjection,
    const glm::vec3& viewPos,
    float time)
{
    if (m_currentIndex >= 0 &&
        m_currentIndex < static_cast<int>(m_listScene.size()))
    {
        m_listScene[m_currentIndex].scene->Render(
            pipeline,
            *this,
            viewMatrix,
            viewProjection,
            viewPos,
            time
        );
    }

    // Toujours dessiner le fondu en dernier.
    RenderFade(pipeline);
}

Scene* SceneManager::GetCurrentScene()
{
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_listScene.size())) {
        return m_listScene[m_currentIndex].scene.get();
    }
    return nullptr;
}

int SceneManager::GetWidth() const
{
    return RENDER_WIDTH;
}

int SceneManager::GetHeight() const
{
    return RENDER_HEIGHT;
}

void SceneManager::SetFadeDuration(float duration)
{
    m_fadeDuration = std::max(0.01f, duration);
}
