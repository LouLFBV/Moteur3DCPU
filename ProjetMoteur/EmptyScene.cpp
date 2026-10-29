#include "EmptyScene.h"
#include "ThirdScene.h"
#include "Rasterizer.h"
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

void EmptyScene::Render(Rasterizer& rasterizer)
{
    float aspect = 320.0f / 200.0f;
    float zNear = 0.1f;
    float zFar = 100.0f;

    // Vue 3D classique (Perspective) : FOV de 60 degrés, ratio de l'écran, near plane à 0.1f, far plane à 100.0f
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, zNear, zFar);

    // FOND + PLAN 

    // 2. FOND (Ciel couvrant tout l'écran, reculé à Z = 0.99f)
    Vertex baseBackground[4] = {
        { { -1.0f,  1.0f, 0.99f, 1.0f }, 0.8f, 0.8f, 0.4f }, // Haut-Gauche (Bleu ciel)
        { { -1.0f, -1.0f, 0.99f, 1.0f }, 1.0f, 0.2f, 1.0f }, // Bas-Gauche (Plus clair)
        { { 1.0f, -1.0f, 0.99f, 1.0f }, 0.8f, 0.8f, 0.4f }, // Bas-Droite
        { { 1.0f,  1.0f, 0.99f, 1.0f }, 1.0f, 0.2f, 1.0f }  // Haut-Droite
    };
    rasterizer.DrawSquare(baseBackground[0], baseBackground[1], baseBackground[2], baseBackground[3]);

    // 3. PLAN / SOL (Couvrant la moitié basse de l'écran)
    Vertex basePlan[4] = {
        { { -1.0f,  0.0f, 0.95f, 1.0f }, 0.4f, 0.4f, 0.4f }, // Ligne d'horizon
        { { -1.0f, -1.0f, 0.95f, 1.0f }, 0.6f, 0.8f, 1.0f }, // Bas de l'écran
        { { 1.0f, -1.0f, 0.95f, 1.0f }, 0.6f, 0.8f, 1.0f },
        { { 1.0f,  0.0f, 0.95f, 1.0f }, 0.2f, 0.4f, 0.8f }
    };
    rasterizer.DrawSquare(basePlan[0], basePlan[1], basePlan[2], basePlan[3]);

    rasterizer.ClearZBuffer();

    //FIN FOND + PLAN
}

void EmptyScene::OnExit()
{
    std::cout << "Scene vide quittee\n";
}