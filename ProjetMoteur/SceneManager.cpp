#include "SceneManager.h"
#include "Rasterizer.h"

void SceneManager::ChangeScene(std::unique_ptr<Scene> newScene)
{
    // On met en attente la nouvelle scène. 
    m_nextScene = std::move(newScene);
}

void SceneManager::ProcessPendingTransition()
{
    if (m_nextScene)
    {
        // 1. Quitter proprement la scène courante
        if (m_currentScene)
        {
            m_currentScene->OnExit();
        }

        // 2. Basculer vers la nouvelle scène
        m_currentScene = std::move(m_nextScene);

        // 3. Initialiser la nouvelle scène
        if (m_currentScene)
        {
            m_currentScene->OnEnter();
        }
    }
}

void SceneManager::HandleEvent(const SDL_Event& event, bool& isPerspective)
{
    if (m_currentScene)
    {
        // On passe à la scène active l'événement SDL, le booléen de perspective 
        m_currentScene->HandleEvent(event, isPerspective, *this);
    }
}

void SceneManager::Update(float deltaTime)
{
    // Sécurité : Vérifie et exécute un changement de scène en attente avant l'Update
    ProcessPendingTransition();

    if (m_currentScene)
    {
        m_currentScene->Update(deltaTime);
    }
}

void SceneManager::Render(Rasterizer& rasterizer)
{
    if (m_currentScene)
    {
        // La scène active soumet ses primitives 3D (DrawTriangle, DrawSquare, DrawCube) 
        m_currentScene->Render(rasterizer);
    }
}