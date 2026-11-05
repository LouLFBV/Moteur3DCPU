#include "SceneManager.h"
#include "Scene.h"

void SceneManager::AddScene(std::unique_ptr<Scene> scene, float t_start, float t_end)
{
    m_listScene.push_back({ std::move(scene), t_start, t_end });

    if (m_currentIndex == -1) {
        SetScene(0);
    }
}

void SceneManager::NextScene()
{
    if (m_listScene.empty()) return;

    // Calcul de l'index suivant avec boucle automatique grâce au modulo %
    size_t nextIndex = m_currentIndex + 1;

    if (nextIndex >= m_listScene.size())
    {
        m_isRunning = false;
        return;
    }

    SetScene(nextIndex);
}

void SceneManager::SetScene(size_t index)
{
    if (index >= m_listScene.size()) return;

    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_listScene.size())) {
        m_listScene[m_currentIndex].scene->OnExit();
    }

    m_currentIndex = static_cast<int>(index);

    //m_listScene[m_currentIndex].scene->OnEnter();
}

void SceneManager::Update(float deltaTime, bool isEditMode)
{
    float effectiveDeltaTime = isEditMode ? 0.0f : deltaTime;

    if (!isEditMode)
    {
        m_globalTime += deltaTime;
    }

    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_listScene.size())) {
        clip& currentClip = m_listScene[m_currentIndex];

        currentClip.scene->Update(effectiveDeltaTime);

        if (!isEditMode)
        {
            float duration = currentClip.t_end - currentClip.t_start;
            if (duration > 0.0f && currentClip.scene->GetSceneTime() >= duration) {
                NextScene();
            }
        }
    }
}

void SceneManager::Render(    GraphicsPipeline& pipeline,    const glm::mat4& viewMatrix,    const glm::mat4& viewProjection,    const glm::vec3& viewPos,    float time)
{
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_listScene.size())) {
        m_listScene[m_currentIndex].scene->Render(pipeline, *this, viewMatrix, viewProjection, viewPos, time);
    }
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
