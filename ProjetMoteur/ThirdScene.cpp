#include "ThirdScene.h"
#include "MainScene.h"
#include "GraphicsPipeline.h"
#include "Camera.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

ThirdScene::ThirdScene(SceneManager& sceneManager) : Scene(sceneManager)
{}

void ThirdScene::OnEnter()
{
    Scene::OnEnter();
    const char* objPath = "external/Assets/catn0.obj";
    const char* texturePath = "external/Assets/cat_text_m.jpeg";

    // On passe le dossier "external/Assets" pour que tinyobj cherche le .mtl au bon endroit
    Get_TOBJ(objPath, m_objModel, "external/Assets");

    m_loaded = !m_objModel.vertices.empty();

    if (m_loaded) {
        std::cout << "Obj Charge depuis : " << objPath << " (" << m_objModel.vertices.size() << " sommets)" << std::endl;

        // Si la texture n'a pas été chargée automatiquement via le .mtl, on la charge à la main :
        if (!m_objModel.hasTexture) {
            if (m_objModel.texture.LoadFromFile(texturePath)) {
                m_objModel.hasTexture = true;
                std::cout << "Texture chargee manuellement : " << texturePath << std::endl;
            }
            else {
                std::cerr << "Erreur de chargement de l'image : " << texturePath << std::endl;
            }
        }
    }
    else {
        std::cerr << "Erreur : Impossible de charger " << objPath << std::endl;
    }
}


void ThirdScene::Update(float deltaTime)
{
    m_time += deltaTime;
}

void ThirdScene::Render(GraphicsPipeline& pipeline, SceneManager& sceneManager, const glm::mat4& viewProjection, const glm::vec3& viewPos, float deltaTime)
{
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
    { { -1.0f,  1.0f, 0.9999f, 1.0f }, 0.02f, 0.02f, 0.10f }, // Haut : Noir / Bleu nuit profond
    { { -1.0f, -1.0f, 0.9999f, 1.0f }, 0.00f, 0.55f, 0.75f }, // Bas  : Cyan lumineux
    { {  1.0f, -1.0f, 0.9999f, 1.0f }, 0.00f, 0.55f, 0.75f }, // Bas  : Cyan lumineux
    { {  1.0f,  1.0f, 0.9999f, 1.0f }, 0.02f, 0.02f, 0.10f }  // Haut : Noir / Bleu nuit profond
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










    // 3. RENDU DU MODÈLE OBJ CHARGÉ
    if (m_loaded) {
        // Fragment Shader pour le Chat (Échantillonnage + Lumière)
        glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, 1.0f, 0.5f));

        pipeline.SetFragmentShader([this, lightDir, viewPos](const BordLigne& pixel)->uint32_t {
            float rTex = 0.8f, gTex = 0.8f, bTex = 0.8f;

            if (m_objModel.hasTexture) {
                uint32_t texColor = m_objModel.texture.Sample(pixel.u, pixel.v);
                rTex = ((texColor >> 16) & 0xFF) / 255.0f;
                gTex = ((texColor >> 8) & 0xFF) / 255.0f;
                bTex = (texColor & 0xFF) / 255.0f;
            }

            // Lumière Blinn-Phong
            glm::vec3 normal = glm::normalize(glm::vec3(pixel.nx, pixel.ny, pixel.nz));
            glm::vec3 pixelPos = glm::vec3(pixel.worldPos.x, pixel.worldPos.y, pixel.worldPos.z);
            glm::vec3 vV = glm::normalize(viewPos - pixelPos);
            glm::vec3 vH = glm::normalize(lightDir + vV);

            float diff = std::max(0.0f, glm::dot(normal, lightDir));
            float spec = std::pow(std::max(0.0f, glm::dot(normal, vH)), 32.0f);
            float lightIntensity = std::min(1.0f, 0.3f + diff + spec * 0.5f);

            uint8_t r = static_cast<uint8_t>(std::clamp(rTex * lightIntensity * 255.0f, 0.0f, 255.0f));
            uint8_t g = static_cast<uint8_t>(std::clamp(gTex * lightIntensity * 255.0f, 0.0f, 255.0f));
            uint8_t b = static_cast<uint8_t>(std::clamp(bTex * lightIntensity * 255.0f, 0.0f, 255.0f));

            return (0xFF << 24) | (r << 16) | (g << 8) | b;
            });

        glm::mat4 modelOBJ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        modelOBJ = glm::scale(modelOBJ, glm::vec3(0.05f, 0.05f, 0.05f));
        modelOBJ = glm::rotate(modelOBJ, deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 mvpOBJ = viewProjection * modelOBJ;

        for (size_t i = 0; i + 2 < m_objModel.vertices.size(); i += 3) {
            auto& src0 = m_objModel.vertices[i + 0];
            auto& src1 = m_objModel.vertices[i + 1];
            auto& src2 = m_objModel.vertices[i + 2];

            Vertex v0 = { src0.position, src0.r, src0.g, src0.b, {src0.tx, src0.ty}, {src0.nx, src0.ny, src0.nz} };
            Vertex v1 = { src1.position, src1.r, src1.g, src1.b, {src1.tx, src1.ty}, {src1.nx, src1.ny, src1.nz} };
            Vertex v2 = { src2.position, src2.r, src2.g, src2.b, {src2.tx, src2.ty}, {src2.nx, src2.ny, src2.nz} };

            pipeline.DrawTriangle(v0, v1, v2, mvpOBJ);
        }

        pipeline.RenderScene();
    }


    pipeline.SetFragmentShader([](const BordLigne& pixel) {
        // On convertit les floats [0.0, 1.0] de BordLigne en entiers [0, 255]
        uint8_t ir = static_cast<uint8_t>(std::clamp(255.0f * pixel.r + 0.5f, 0.0f, 255.0f));
        uint8_t ig = static_cast<uint8_t>(std::clamp(255.0f * pixel.g + 0.5f, 0.0f, 255.0f));
        uint8_t ib = static_cast<uint8_t>(std::clamp(255.0f * pixel.b + 0.5f, 0.0f, 255.0f));

        // On assemble le tout au format 32-bit (Alpha, Rouge, Vert, Bleu)
        return (0xFF << 24) | (ir << 16) | (ig << 8) | ib;
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

    // 4. CUBE 3D 1w

    glm::mat4 model1 = glm::mat4(1.0f);
    model1 = glm::translate(model1, glm::vec3(3.0f, 0.0f, -4.5f));
    model1 = glm::rotate(model1, deltaTime, glm::vec3(1.0f, 0.7f, 0.0f));
    glm::mat4 mvp1 = viewProjection * model1;

    Cube cubeFinal1 = {
        baseCube[0], baseCube[1], baseCube[2], baseCube[3],
        baseCube[7], baseCube[6], baseCube[5], baseCube[4],
        baseCube[4], baseCube[5], baseCube[1], baseCube[0],
        baseCube[3], baseCube[2], baseCube[6], baseCube[7],
        baseCube[4], baseCube[0], baseCube[3], baseCube[7],
        baseCube[1], baseCube[5], baseCube[6], baseCube[2]
    };
    pipeline.DrawCube(cubeFinal1, mvp1);

    pipeline.RenderScene();

}

void ThirdScene::OnExit()
{
    m_objModel.vertices.clear();
    std::cout << "Troisieme Scene quittee\n";
}