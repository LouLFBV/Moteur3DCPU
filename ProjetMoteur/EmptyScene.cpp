#include "EmptyScene.h"
#include "ThirdScene.h"
#include "GraphicsPipeline.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void EmptyScene::OnEnter()
{
    std::cout << "Scene vide\n";
}

void EmptyScene::HandleEvent(const SDL_Event& event, bool& isPerspective, SceneManager& sceneManager)
{
    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
        if (event.key.key == SDLK_T) {
            std::cout << "Scene Changer" << std::endl;
            sceneManager.ChangeScene(std::make_unique<ThirdScene>());
        }
    }
}

void EmptyScene::Update(float deltaTime)
{
    m_time += deltaTime;
}

void EmptyScene::Render(GraphicsPipeline& pipeline)
{
    std::cout << "Render Empty Scene appelé" << std::endl;
}

void EmptyScene::OnExit()
{
    std::cout << "Scene vide quittee\n";
}