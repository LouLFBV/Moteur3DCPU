#include "EmptyScene.h"
#include "ThirdScene.h"
#include "GraphicsPipeline.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>

EmptyScene::EmptyScene(SceneManager& sceneManager) : Scene(sceneManager)
{}

void EmptyScene::OnEnter()
{
    Scene::OnEnter();
    std::cout << "Scene vide\n";
}


void EmptyScene::Update(float deltaTime)
{
    m_time += deltaTime;
}

void EmptyScene::Render(GraphicsPipeline& pipeline, SceneManager& sceneManager, const glm::mat4& viewProjection, const glm::vec3& viewPos, float deltaTime)
{

    pipeline.ResetStats();
    pipeline.Clear();



    glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, 1.0f, 0.5f));


    // RESET DE FRAGMENT SHADER 
    pipeline.SetFragmentShader([](const BordLigne& pixel) {
        // On convertit les floats [0.0, 1.0] de BordLigne en entiers [0, 255]
        uint8_t ir = static_cast<uint8_t>(std::clamp(255.0f * pixel.r + 0.5f, 0.0f, 255.0f));
        uint8_t ig = static_cast<uint8_t>(std::clamp(255.0f * pixel.g + 0.5f, 0.0f, 255.0f));
        uint8_t ib = static_cast<uint8_t>(std::clamp(255.0f * pixel.b + 0.5f, 0.0f, 255.0f));

        // On assemble le tout au format 32-bit (Alpha, Rouge, Vert, Bleu)
        return (0xFF << 24) | (ir << 16) | (ig << 8) | ib;
        });



    // 1. FOND (Ciel en NDC)
    Vertex baseBackground[4] = {
     { { -1.0f,  1.0f, 0.9999f, 1.0f }, 0.25f, 0.08f, 0.35f }, // Haut : Violet crépuscule
     { { -1.0f, -1.0f, 0.9999f, 1.0f }, 0.95f, 0.45f, 0.20f }, // Bas  : Orange chaud
     { {  1.0f, -1.0f, 0.9999f, 1.0f }, 0.95f, 0.45f, 0.20f }, // Bas  : Orange chaud
     { {  1.0f,  1.0f, 0.9999f, 1.0f }, 0.25f, 0.08f, 0.35f }  // Haut : Violet crépuscule
    };
    pipeline.DrawSquareNDC(baseBackground[0], baseBackground[1], baseBackground[2], baseBackground[3]);

    // 2. SOL (En NDC)
    Vertex basePlan[4] = {
        { { -1.0f,  0.0f, 0.999f, 1.0f }, 0.4f, 0.4f, 0.4f },
        { { -1.0f, -1.0f, 0.999f, 1.0f }, 0.2f, 0.2f, 0.2f },
        { {  1.0f, -1.0f, 0.999f, 1.0f }, 0.2f, 0.2f, 0.2f },
        { {  1.0f,  0.0f, 0.999f, 1.0f }, 0.4f, 0.4f, 0.4f }
    };
    pipeline.DrawSquareNDC(basePlan[0], basePlan[1], basePlan[2], basePlan[3]);

    pipeline.ClearZBuffer();

    pipeline.RenderScene();


    pipeline.SetFragmentShader([deltaTime](const BordLigne& pixel) {
        //float time = SDL_GetTicks() * 0.003f;

        // On combine x et z (profondeur) avec le temps pour faire une onde
        float wave = std::sin(pixel.x * 0.05f + pixel.z * 15.0f + deltaTime);
        float intensity = (wave * 0.5f) + 0.5f; // Ramène entre 0.0 et 1.0

        // Dégradé Cyan / Violet cyberpunk
        uint8_t r = static_cast<uint8_t>(intensity * 255.0f);
        uint8_t g = static_cast<uint8_t>((1.0f - intensity) * 150.0f);
        uint8_t b = 255;

        return (0xFF << 24) | (r << 16) | (g << 8) | b;
        });


    // 6. TRIANGLE 3D SUPPLÉMENTAIRE
    Vertex verticesTri[3] = {
        { {  0.0f,  0.4f, 0.0f, 1.0f}, 1.0f, 0.0f, 0.0f },
        { { -0.4f, -0.4f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f },
        { {  0.4f, -0.4f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f }
    };
    glm::mat4 model3 = glm::mat4(1.0f);
    model3 = glm::translate(model3, glm::vec3(-1.0f, 0.0f, -1.0f));
    model3 = glm::rotate(model3, deltaTime * 1.5f, glm::vec3(-1.0f, 0.0f, 0.0f));
    glm::mat4 mvp3 = viewProjection * model3;

    pipeline.DrawTriangle(verticesTri[0], verticesTri[1], verticesTri[2], mvp3);

    pipeline.RenderScene();

    pipeline.SetFragmentShader([](const BordLigne& pixel) {
        // On convertit les floats [0.0, 1.0] de BordLigne en entiers [0, 255]
        uint8_t ir = static_cast<uint8_t>(std::clamp(255.0f * pixel.r + 0.5f, 0.0f, 255.0f));
        uint8_t ig = static_cast<uint8_t>(std::clamp(255.0f * pixel.g + 0.5f, 0.0f, 255.0f));
        uint8_t ib = static_cast<uint8_t>(std::clamp(255.0f * pixel.b + 0.5f, 0.0f, 255.0f));

        // On assemble le tout au format 32-bit (Alpha, Rouge, Vert, Bleu)
        return (0xFF << 24) | (ir << 16) | (ig << 8) | ib;
        });

    float smoothness = 32.0f;

    pipeline.SetFragmentShader([lightDir, viewPos, smoothness](const BordLigne& pixel) {

            glm::vec3 normal = glm::normalize(glm::vec3(pixel.nx, pixel.ny, pixel.nz));

            glm::vec3 vL = lightDir;

            glm::vec3 pixelPos = glm::vec3(pixel.worldPos.x, pixel.worldPos.y, pixel.worldPos.z);
            glm::vec3 vV = glm::normalize(viewPos - pixelPos);

            glm::vec3 vH = glm::normalize(vL + vV);

            float diff = std::max(0.0f, glm::dot(normal, vL));

            float specBase = std::max(0.0f, glm::dot(normal, vH));
            float spec = std::pow(specBase, smoothness); 

            float ambient = 0.4f;
            float lightIntensity = std::min(1.0f, ambient + diff + (spec * 0.8f));

            // G. Application sur la couleur de base
            uint8_t r = static_cast<uint8_t>(std::clamp(255.0f * pixel.r * lightIntensity + 0.5f, 0.0f, 255.0f));
            uint8_t g = static_cast<uint8_t>(std::clamp(255.0f * pixel.g * lightIntensity + 0.5f, 0.0f, 255.0f));
            uint8_t b = static_cast<uint8_t>(std::clamp(255.0f * pixel.b * lightIntensity + 0.5f, 0.0f, 255.0f));

            return (0xFF << 24) | (r << 16) | (g << 8) | b;
            });

    // Données des cubes
    Vertex baseCube[8] = {
        // Position                     // R,    G,    B     // UV           // Normale (x, y, z)
        { { -0.5f,  0.5f,  0.5f, 1.0f }, 1.0f, 0.0f, 0.0f, { 0.0f, 0.0f }, {-0.577f,  0.577f,  0.577f} },
        { { -0.5f, -0.5f,  0.5f, 1.0f }, 0.0f, 1.0f, 0.0f, { 0.0f, 1.0f }, {-0.577f, -0.577f,  0.577f} },
        { {  0.5f, -0.5f,  0.5f, 1.0f }, 0.0f, 0.0f, 1.0f, { 1.0f, 1.0f }, { 0.577f, -0.577f,  0.577f} },
        { {  0.5f,  0.5f,  0.5f, 1.0f }, 1.0f, 1.0f, 0.0f, { 1.0f, 0.0f }, { 0.577f,  0.577f,  0.577f} },
        { { -0.5f,  0.5f, -0.5f, 1.0f }, 0.0f, 1.0f, 1.0f, { 1.0f, 0.0f }, {-0.577f,  0.577f, -0.577f} },
        { { -0.5f, -0.5f, -0.5f, 1.0f }, 1.0f, 0.0f, 1.0f, { 1.0f, 1.0f }, {-0.577f, -0.577f, -0.577f} },
        { {  0.5f, -0.5f, -0.5f, 1.0f }, 1.0f, 1.0f, 1.0f, { 0.0f, 1.0f }, { 0.577f, -0.577f, -0.577f} },
        { {  0.5f,  0.5f, -0.5f, 1.0f }, 0.5f, 0.5f, 0.5f, { 0.0f, 0.0f }, { 0.577f,  0.577f, -0.577f} }
    };


    // 5. CUBE 3D 2
    glm::mat4 model2 = glm::mat4(1.0f);
    model2 = glm::translate(model2, glm::vec3(1.0f, 0.0f, -3.5f));
    model2 = glm::rotate(model2, deltaTime * 2.f, glm::vec3(-1.0f, 0.7f, 0.0f));
    glm::mat4 mvp2 = viewProjection * model2;

    Cube cubeFinal2 = {
        baseCube[0], baseCube[1], baseCube[2], baseCube[3],
        baseCube[7], baseCube[6], baseCube[5], baseCube[4],
        baseCube[4], baseCube[5], baseCube[1], baseCube[0],
        baseCube[3], baseCube[2], baseCube[6], baseCube[7],
        baseCube[4], baseCube[0], baseCube[3], baseCube[7],
        baseCube[1], baseCube[5], baseCube[6], baseCube[2]
    };
    pipeline.DrawCube(cubeFinal2, mvp2);

    pipeline.RenderScene();
}

void EmptyScene::OnExit()
{
    std::cout << "Scene vide quittee\n";
}