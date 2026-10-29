#pragma once

#include "SceneManager.h"

class EmptyScene : public Scene
{
public:
    EmptyScene() = default;
    ~EmptyScene() override = default;

    void OnEnter() override;
    void HandleEvent(const SDL_Event& event, bool& isPerspective, SceneManager& sceneManager) override;
    void Update(float deltaTime) override;
    void Render(Rasterizer& rasterizer) override;
    void OnExit() override;

private:
    float m_time = 0.0f;
};