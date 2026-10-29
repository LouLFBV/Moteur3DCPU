#pragma once

#include "SceneManager.h"
#include "ObjLoader.h"

class MainScene : public Scene
{
public:
    MainScene() = default;
    ~MainScene() override = default;

    void OnEnter() override;
    void HandleEvent(const SDL_Event& event, bool& isPerspective, SceneManager& sceneManager) override;
    void Update(float deltaTime) override;
    void Render(GraphicsPipeline& rasterizer) override;
    void OnExit() override;

private:
    T_OBJ m_objModel;
    bool m_loaded = false;
    float m_time = 0.0f;
};