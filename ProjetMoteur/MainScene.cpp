#include "MainScene.h"
#include "EmptyScene.h"
#include "GraphicsPipeline.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void MainScene::OnEnter()
{
    std::cout << "Scene Principale\n";
    // Chargement Obj
    const char* objPath = "external/Assets/base.obj";
    Get_TOBJ(objPath, m_objModel);
    m_loaded = !m_objModel.vertices.empty();

    if (m_loaded) {
        std::cout << "Obj Charge depuis : " << objPath << " (" << m_objModel.vertices.size() << " sommets)" << std::endl;
    }
    else {
        std::cerr << "Erreur : Impossible de charger " << objPath << std::endl;
    }
    // Fin Chargement Obj
}

void MainScene::HandleEvent(const SDL_Event& event, bool& isPerspective, SceneManager& sceneManager)
{
    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
        if (event.key.key == SDLK_T) {
            std::cout << "Scene Changer" << std::endl;
            sceneManager.ChangeScene(std::make_unique<EmptyScene>());
        }
    }
}

void MainScene::Update(float deltaTime)
{
    m_time += deltaTime;
}

void MainScene::Render(GraphicsPipeline& pipeline)
{
    std::cout << "Render main scene appelé"<< std::endl;
}

void MainScene::OnExit()
{
    m_objModel.vertices.clear();
    std::cout << "Scene Principale quittee\n";
}