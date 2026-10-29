#pragma once

#include <memory>
#include <SDL3/SDL.h>

class Rasterizer;
class SceneManager;

class Scene
{
public:
    virtual ~Scene() = default;

    virtual void OnEnter() = 0;
    virtual void HandleEvent(const SDL_Event& event, bool& isPerspective, SceneManager& sceneManager) = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render(Rasterizer& rasterizer) = 0;
    virtual void OnExit() = 0;
};

class SceneManager
{
public:
    SceneManager() = default;
    ~SceneManager() = default;

    // Empêche la copie pour éviter la duplication d'états de scènes
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    // Demande de changement de scène
    void ChangeScene(std::unique_ptr<Scene> newScene);

    void HandleEvent(const SDL_Event& event, bool& isPerspective);
    void Update(float deltaTime);
    void Render(Rasterizer& rasterizer);

private:
    // Exécute la transition en attente en début d'update
    void ProcessPendingTransition();

    std::unique_ptr<Scene> m_currentScene = nullptr;
    std::unique_ptr<Scene> m_nextScene = nullptr;
};