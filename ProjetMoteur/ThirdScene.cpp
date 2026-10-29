#include "ThirdScene.h"
#include "MainScene.h"
#include "Rasterizer.h"
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

void ThirdScene::Render(Rasterizer& rasterizer)
{
    float aspect = 320.0f / 200.0f;
    float zNear = 0.1f;
    float zFar = 100.0f;

    // Vue 3D classique (Perspective) : FOV de 60 degrés, ratio de l'écran, near plane à 0.1f, far plane à 100.0f
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, zNear, zFar);


    // FOND + PLAN 

    // 2. FOND (Ciel couvrant tout l'écran, reculé à Z = 0.99f)
    Vertex baseBackground[4] = {
        { { -1.0f,  1.0f, 0.99f, 1.0f }, 0.1f, 0.2f, 1.0f }, // Haut-Gauche (Bleu ciel)
        { { -1.0f, -1.0f, 0.99f, 1.0f }, 1.0f, 0.6f, 0.2f }, // Bas-Gauche (Plus clair)
        { { 1.0f, -1.0f, 0.99f, 1.0f }, 0.8f, 0.8f, 1.0f }, // Bas-Droite
        { { 1.0f,  1.0f, 0.99f, 1.0f }, 0.6f, 0.6f, 1.0f }  // Haut-Droite
    };
    rasterizer.DrawSquare(baseBackground[0], baseBackground[1], baseBackground[2], baseBackground[3]);

    // 3. PLAN / SOL (Couvrant la moitié basse de l'écran)
    Vertex basePlan[4] = {
        { { -1.0f,  0.0f, 0.95f, 1.0f }, 0.8f, 0.2f, 0.4f }, // Ligne d'horizon
        { { -1.0f, -1.0f, 0.95f, 1.0f }, 0.6f, 0.8f, 0.8f }, // Bas de l'écran
        { { 1.0f, -1.0f, 0.95f, 1.0f }, 0.6f, 0.8f, 1.0f },
        { { 1.0f,  0.0f, 0.95f, 1.0f }, 0.8f, 0.8f, 1.0f }
    };
    rasterizer.DrawSquare(basePlan[0], basePlan[1], basePlan[2], basePlan[3]);

    rasterizer.ClearZBuffer();

    //FIN FOND + PLAN


    // RENDU DU MODÈLE OBJ CHARGÉ
    if (m_loaded) {
        glm::mat4 modelOBJ = glm::mat4(1.0f);
        modelOBJ = glm::translate(modelOBJ, glm::vec3(-2.0f, 0.0f, -3.0f)); // Positionne le modèle devant la caméra
        modelOBJ = glm::rotate(modelOBJ, m_time, glm::vec3(0.0f, 1.0f, 1.0f));  // Rotation continue autour de Y
        modelOBJ = glm::scale(modelOBJ, glm::vec3(1.0f));                     // Ajuste l'échelle si besoin

        glm::mat4 transformOBJ = projection * modelOBJ;

        for (size_t i = 0; i + 2 < m_objModel.vertices.size(); i += 3) {
            Vertex v0 = { m_objModel.vertices[i + 0].position, m_objModel.vertices[i + 0].r, m_objModel.vertices[i + 0].g, m_objModel.vertices[i + 0].b };
            Vertex v1 = { m_objModel.vertices[i + 1].position, m_objModel.vertices[i + 1].r, m_objModel.vertices[i + 1].g, m_objModel.vertices[i + 1].b };
            Vertex v2 = { m_objModel.vertices[i + 2].position, m_objModel.vertices[i + 2].r, m_objModel.vertices[i + 2].g, m_objModel.vertices[i + 2].b };

            // Application des matrices GLM (Model + Projection)
            v0.position = transformOBJ * v0.position;
            v1.position = transformOBJ * v1.position;
            v2.position = transformOBJ * v2.position;

            rasterizer.DrawTriangle(v0, v1, v2);
        }
    }
    // FIN RENDU DU MODÈLE OBJ CHARGÉ

    // DESSINER UN CUBE EN 3D 

    // Les 8 sommets d'un cube centré sur 0
    Vertex baseCube[8] = {
        // Face avant (Z = 0.5)
        { { -0.5f,  0.5f,  0.5f, 1.0f }, 1.0f, 0.0f, 0.0f }, // 0: Haut-Gauche (Rouge)
        { { -0.5f, -0.5f,  0.5f, 1.0f }, 0.0f, 1.0f, 0.0f }, // 1: Bas-Gauche (Vert)
        { { 0.5f, -0.5f,  0.5f, 1.0f }, 0.0f, 0.0f, 1.0f }, // 2: Bas-Droite (Bleu)
        { { 0.5f,  0.5f,  0.5f, 1.0f }, 1.0f, 1.0f, 0.0f }, // 3: Haut-Droite (Jaune)
        // Face arrière (Z = -0.5)
        { { -0.5f,  0.5f, -0.5f, 1.0f }, 0.0f, 1.0f, 1.0f }, // 4: Haut-Gauche arr (Cyan)
        { { -0.5f, -0.5f, -0.5f, 1.0f }, 1.0f, 0.0f, 1.0f }, // 5: Bas-Gauche arr (Magenta)
        { { 0.5f, -0.5f, -0.5f, 1.0f }, 1.0f, 1.0f, 1.0f }, // 6: Bas-Droite arr (Blanc)
        { { 0.5f,  0.5f, -0.5f, 1.0f }, 0.5f, 0.5f, 0.5f }  // 7: Haut-Droite arr (Gris)
    };

    // --- LE CUBE 3D 1 ---
    // 1. On crée une matrice d'identité
    glm::mat4 model = glm::mat4(1.0f);

    // 2. On recule le cube pour le voir
    model = glm::translate(model, glm::vec3(1.0f, 0.0f, -3.5f));

    // 3. On le fait tourner sur les axes X et Y en même temps 
    model = glm::rotate(model, m_time, glm::vec3(1.0f, 0.7f, 0.0f));

    // 4. On combine la Perspective et le Model
    glm::mat4 transform = projection * model;

    // 5. On applique cette super-matrice à nos 8 sommets
    Vertex cube[8];
    for (int i = 0; i < 8; ++i) {
        cube[i] = baseCube[i];
        // La matrice s'occupe de la rotation, du recul ET de la perspective !
        cube[i].position = transform * cube[i].position;
    }

    Cube cubeFinal = {
        cube[0], cube[1], cube[2], cube[3],
        cube[7], cube[6], cube[5], cube[4],
        cube[4], cube[5], cube[1], cube[0],
        cube[3], cube[2], cube[6], cube[7],
        cube[4], cube[0], cube[3], cube[7],
        cube[1], cube[5], cube[6], cube[2]
    };

    rasterizer.DrawCube(cubeFinal);


    // --- LE CUBE 3D 2 ---
    // 1. On crée une matrice d'identité
    glm::mat4 model2 = glm::mat4(1.0f);

    // 2. On recule le cube pour le voir
    model2 = glm::translate(model2, glm::vec3(0.0f, 0.0f, -2.5f));

    // 3. On le fait tourner sur les axes X et Y en même temps 
    model2 = glm::rotate(model2, m_time * 2.f, glm::vec3(-1.0f, 0.7f, 0.0f));

    // 4. On combine la Perspective et le Model
    glm::mat4 transform2 = projection * model2;

    // 5. On applique cette super-matrice à nos 8 sommets
    Vertex cube2[8];
    for (int i = 0; i < 8; ++i) {
        cube2[i] = baseCube[i];
        // La matrice s'occupe de la rotation, du recul ET de la perspective !
        cube2[i].position = transform2 * cube2[i].position;
    }

    Cube cubeFinal2 = {
        cube2[0], cube2[1], cube2[2], cube2[3],
        cube2[7], cube2[6], cube2[5], cube2[4],
        cube2[4], cube2[5], cube2[1], cube2[0],
        cube2[3], cube2[2], cube2[6], cube2[7],
        cube2[4], cube2[0], cube2[3], cube2[7],
        cube2[1], cube2[5], cube2[6], cube2[2]
    };

    rasterizer.DrawCube(cubeFinal2);


    // TRIANGLE  2D EN PLUS 
    Vertex verticesTri[3] = {
        { { 0.0f,  0.4f, -.0f, 1.0f }, 1.0f, 0.0f, 0.0f },
        { { -0.4f, -0.4f, -0.0f, 1.0f }, 0.0f, 1.0f, 0.0f },
        { { 0.4f, -0.4f, -.0f, 1.0f }, 0.0f, 0.0f, 1.0f }
    };

    // 1. On crée une matrice d'identité
    glm::mat4 model3 = glm::mat4(1.0f);

    // 2. On recule le cube pour le voir
    model3 = glm::translate(model3, glm::vec3(-1.0f, 0.0f, -1.0f));

    // 3. On le fait tourner sur les axes X et Y en même temps 
    model3 = glm::rotate(model3, m_time * 1.5f, glm::vec3(-1.0f, -0.f, 0.0f));

    // 4. On combine la Perspective et le Model
    glm::mat4 transform3 = projection * model3;

    // 5. On applique cette super-matrice à nos 3 sommets
    Vertex triangle[3];
    for (int i = 0; i < 3; ++i) {
        triangle[i] = verticesTri[i];
        // La matrice s'occupe de la rotation, du recul ET de la perspective !
        triangle[i].position = transform3 * verticesTri[i].position;
    }

    rasterizer.DrawTriangle(triangle[0], triangle[1], triangle[2]);

    //FIN DESSINER UN CUBE EN 3D

}

void ThirdScene::OnExit()
{
    m_objModel.vertices.clear();
    std::cout << "Troisieme Scene quittee\n";
}