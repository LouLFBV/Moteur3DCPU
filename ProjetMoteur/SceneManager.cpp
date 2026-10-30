#include "SceneManager.h"
#include "Scene.h"

void SceneManager::AddScene(std::unique_ptr<Scene> scene)
{
    m_listScene.push_back(std::move(scene));

    if (m_currentIndex == -1) {
        SetScene(0);
    }
}

void SceneManager::NextScene()
{
    if (m_listScene.empty()) return;

    // Calcul de l'index suivant avec boucle automatique grâce au modulo %
    size_t nextIndex = (m_currentIndex + 1) % m_listScene.size();
    SetScene(nextIndex);
}

void SceneManager::SetScene(size_t index)
{
    if (index >= m_listScene.size()) return;

    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_listScene.size())) {
        m_listScene[m_currentIndex]->OnExit();
    }

    m_currentIndex = static_cast<int>(index);

    m_listScene[m_currentIndex]->OnEnter();
}

void SceneManager::Update(float deltaTime)
{
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_listScene.size())) {
        m_listScene[m_currentIndex]->Update(deltaTime);
    }
}

void SceneManager::Render(GraphicsPipeline& pipeline, const glm::mat4& viewProj, const glm::vec3& viewPos, float time)
{
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_listScene.size())) {
        m_listScene[m_currentIndex]->Render(pipeline, *this, viewProj, viewPos, time);
    }
}

Scene* SceneManager::GetCurrentScene()
{
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_listScene.size())) {
        return m_listScene[m_currentIndex].get();
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
