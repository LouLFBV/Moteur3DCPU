#include "ThirdScene.h"
#include "MainScene.h"
#include "GraphicsPipeline.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void ThirdScene::OnEnter()
{
    std::cout << "Troisieme Scene\n";
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

void ThirdScene::HandleEvent(const SDL_Event& event, bool& isPerspective, SceneManager& sceneManager)
{
    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
        if (event.key.key == SDLK_T) {
            std::cout << "Scene Changer" << std::endl;
            sceneManager.ChangeScene(std::make_unique<MainScene>());
        }
    }
}

void ThirdScene::Update(float deltaTime)
{
    m_time += deltaTime;
}

void ThirdScene::Render(GraphicsPipeline& pipeline)
{
    std::cout << "Render Third Scene.\n";
}

void ThirdScene::OnExit()
{
    m_objModel.vertices.clear();
    std::cout << "Troisieme Scene quittee\n";
}