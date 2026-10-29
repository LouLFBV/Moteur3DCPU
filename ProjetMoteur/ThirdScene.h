#pragma once

#include "SceneManager.h"
#include "ObjLoader.h"

class ThirdScene : public Scene
{
public:
    ThirdScene() = default;
    ~ThirdScene() override = default;

    void OnEnter() override;
    void HandleEvent(const SDL_Event& event, bool& isPerspective, SceneManager& sceneManager) override;
    void Update(float deltaTime) override;
    void Render(Rasterizer& rasterizer) override;
    void OnExit() override;

private:
    T_OBJ m_objModel;
    bool m_loaded = false;
    float m_time = 0.0f;
};