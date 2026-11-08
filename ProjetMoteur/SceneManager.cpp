#include "SceneManager.h"
#include "GraphicsPipeline.h"
#include <algorithm>
#include <utility>

SceneManager::SceneManager()
{
    SetFadeDuration(1.0f);
}

void SceneManager::AddScene(std::unique_ptr<Scene> scene, float t_start, float t_end)
{
    m_listScene.push_back({
        std::move(scene),
        t_start,
        t_end
        });

    // La première scène ajoutée devient la scène active.
    if (m_currentIndex == -1)
    {
        SetScene(0);
    }
}

void SceneManager::NextScene(bool isEditMode)
{
    if (m_listScene.empty()) {
        return;
    }

    const std::size_t nextIndex = static_cast<std::size_t>(m_currentIndex + 1);

    // Arrêt du moteur lorsqu'il n'y a plus de scène.
    if (nextIndex >= m_listScene.size()) {
        m_isRunning = false;
        return;
    }

    SetScene(nextIndex, isEditMode);
}

void SceneManager::SetScene(std::size_t index, bool isEditMode)
{
    if (index >= m_listScene.size()) {
        return;
    }

    // En mode édition, le changement de scène est immédiat.
    // La première scène est également activée immédiatement.
    if (m_currentIndex == -1 || isEditMode) {
        ActivateScene(index);

        m_fadeAlpha = 1.0f;
        m_fadeTimer = 0.0f;
        m_fadeState = FadeState::FadeIn;

        return;
    }

    if (static_cast<std::size_t>(m_currentIndex) == index) {
        return;
    }

    // Mémorisation de la prochaine scène.
    m_pendingSceneIndex = static_cast<int>(index);

    // Démarrage du Fade Out.
    m_fadeTimer = 0.0f;
    m_fadeAlpha = 0.0f;
    m_fadeState = FadeState::FadeOut;
}

void SceneManager::ActivateScene(std::size_t index)
{
    if (index >= m_listScene.size()) {
        return;
    }

    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_listScene.size())) {
        m_listScene[m_currentIndex].scene->OnExit();
    }

    m_currentIndex = static_cast<int>(index);
}

void SceneManager::UpdateFade(float deltaTime)
{
    if (m_fadeState == FadeState::None) {
        return;
    }

    m_fadeTimer += deltaTime;

    const float progress = std::clamp(
        m_fadeTimer / m_fadeDuration,
        0.0f,
        1.0f
    );

    const float eased = progress * progress * (3.0f - 2.0f * progress);

    switch (m_fadeState) {
        case FadeState::FadeOut:
        {
            m_fadeAlpha = eased;

            if (progress >= 1.0f) {
                if (m_pendingSceneIndex >= 0) {
                    ActivateScene( static_cast<std::size_t>(m_pendingSceneIndex));
                    m_pendingSceneIndex = -1;
                }

                m_fadeTimer = 0.0f;
                m_fadeState = FadeState::FadeIn;
            }
            break;
        }

        case FadeState::FadeIn:
        {
            m_fadeAlpha = 1.0f - eased;

            if (progress >= 1.0f) {
                m_fadeAlpha = 0.0f;
                m_fadeState = FadeState::None;
            }
            break;
        }

        case FadeState::None:
            break;
        }
}

void SceneManager::RenderFade(GraphicsPipeline& pipeline) const
{
    if (m_fadeAlpha <= 0.0f) {
        return;
    }

    // Application du fondu sur l'image calculée.
    pipeline.DrawFadeOverlay(m_fadeAlpha, m_fadeColor);
}

void SceneManager::Update(float deltaTime, bool isEditMode)
{
    const float effectiveDeltaTime = isEditMode ? 0.0f : deltaTime;

    if (!isEditMode) {
        m_globalTime += deltaTime;
    }

    UpdateFade(deltaTime);

    if (m_currentIndex < 0 || m_currentIndex >= static_cast<int>(m_listScene.size())) {
        return;
    }

    SceneEntry& currentScene = m_listScene[m_currentIndex];

    currentScene.scene->Update(effectiveDeltaTime);

    if (isEditMode || m_fadeState != FadeState::None) {
        return;
    }

    const float duration = currentScene.t_end - currentScene.t_start;

    if (duration > 0.0f && currentScene.scene->GetSceneTime() >= duration) {
        NextScene(isEditMode);
    }
}

void SceneManager::Render( GraphicsPipeline& pipeline, const glm::mat4& viewMatrix, const glm::mat4& viewProjection, const glm::vec3& viewPos, float time)
{
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_listScene.size())) {
        m_listScene[m_currentIndex].scene->Render(
            pipeline,
            *this,
            viewMatrix,
            viewProjection,
            viewPos,
            time
        );
    }

    // Le fondu est toujours dessiné en dernier.
    RenderFade(pipeline);
}

Scene* SceneManager::GetCurrentScene()
{
    if (m_currentIndex < 0 || m_currentIndex >= static_cast<int>(m_listScene.size())) {
        return nullptr;
    }

    return m_listScene[m_currentIndex].scene.get();
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
