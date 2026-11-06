#include "LastScene.h"
#include "GraphicsPipeline.h"
#include "Camera.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "MaterialPresets.h"
#include "SceneManager.h"

// =====================================================
// CONSTRUCTEUR
// =====================================================
LastScene::LastScene(SceneManager& sceneManager) : Scene(sceneManager)
{
    m_gameObjects.clear();

    const char* villageDir = "external/Assets/Village";

    // 1. CHOUETTE / POV (Index 0)
    GameObject owl;
    owl.transform.scale = glm::vec3(0.001f);
    if (LoadGameObject(owl, "external/Assets/Village/owl.obj", "external/Assets/Village/wild_animals_map.png", villageDir)) {
        m_gameObjects.push_back(owl);
    }

    // 2. FEU DE CAMP (Index 1)
    GameObject campfire;
    campfire.transform.translation = glm::vec3(0.0f, -1.15f, -3.8f);
    campfire.transform.scale = glm::vec3(1.0f);
    campfire.transform.rotation = glm::vec3(0.0f);
    if (LoadGameObject(campfire, "external/Assets/Village/campfire_pr.obj", nullptr, villageDir)) {
        m_gameObjects.push_back(campfire);
    }

    // 3. MAISON (Index 2 : Avec Shader rigolo)
    GameObject house;
    house.transform.translation = glm::vec3(-4.8f, -1.45f, -7.5f);
    house.transform.rotation = glm::vec3(0.0f, glm::radians(35.0f), 0.0f);
    house.transform.scale = glm::vec3(0.001f);
    if (LoadGameObject(house, "external/Assets/Village/house_type_02_FBX.obj", nullptr, villageDir)) {
        m_gameObjects.push_back(house);
    }

    // 4. ARBRES DE LA FORÊT (Index 3 à 6)
    struct TreeTransform {
        const char* modelFile;
        glm::vec3 pos;
        float scale;
        float rotY;
    };

    TreeTransform trees[4] = {
        { "external/Assets/Village/Low_Poly_Forest_treeTall01.obj", glm::vec3(-6.2f, -1.75f, -4.5f), 0.004f,   15.0f },
        { "external/Assets/Village/Low_Poly_Forest_treeTall02.obj", glm::vec3(4.8f, -1.45f, -6.0f), 0.005f,  -40.0f },
        { "external/Assets/Village/Low_Poly_Forest_treeTall01.obj", glm::vec3(-2.8f, -1.55f, -9.5f), 0.0045f,  90.0f },
        { "external/Assets/Village/Low_Poly_Forest_treeTall02.obj", glm::vec3(3.8f, -1.55f, -9.0f), 0.0055f, 120.0f }
    };

    for (int i = 0; i < 4; ++i) {
        GameObject tree;
        tree.transform.translation = trees[i].pos;
        tree.transform.rotation = glm::vec3(0.0f, glm::radians(trees[i].rotY), 0.0f);
        tree.transform.scale = glm::vec3(trees[i].scale);

        if (LoadGameObject(tree, trees[i].modelFile, "external/Assets/Village/treeTall.png", villageDir)) {
            m_gameObjects.push_back(tree);
        }
    }

    // 5. OURS ET LOUPS (Index 7 à 9)
    struct AnimalTransform {
        const char* modelFile;
        glm::vec3 pos;
        float scale;
        float rotY;
    };

    AnimalTransform animals[3] = {
        { "external/Assets/Village/bear.obj", glm::vec3(3.0f, -1.07f, -4.8f), 1.0f, -65.0f },
        { "external/Assets/Village/wolf.obj", glm::vec3(-2.2f, -1.65f, -3.6f), 0.0012f,  30.0f },
        { "external/Assets/Village/wolf.obj", glm::vec3(1.4f, -1.f, -7.2f), 0.0012f, 150.0f }
    };

    for (int i = 0; i < 3; ++i) {
        GameObject animal;
        animal.transform.translation = animals[i].pos;
        animal.transform.rotation = glm::vec3(0.0f, glm::radians(animals[i].rotY), 0.0f);
        animal.transform.scale = glm::vec3(animals[i].scale);

        if (LoadGameObject(animal, animals[i].modelFile, "external/Assets/Village/wild_animals_map.png", villageDir)) {
            m_gameObjects.push_back(animal);
        }
    }

    // 6. SCÈNE FIXE : PAS DE KEYFRAMES D'AVANCEMENT SUR CETTE SCÈNE

    // 7. GÉNÉRATION DU TERRAIN (HERBE VERTE)
    m_terrainVertices.clear();
    float halfSize = (m_gridSize * m_cellSize) * 0.5f;

    for (int z = 0; z < m_gridSize; ++z) {
        for (int x = 0; x < m_gridSize; ++x) {
            float x0 = x * m_cellSize - halfSize;
            float z0 = z * m_cellSize - halfSize;
            float x1 = (x + 1) * m_cellSize - halfSize;
            float z1 = (z + 1) * m_cellSize - halfSize;

            float u0 = (float)x / m_gridSize, v0 = (float)z / m_gridSize;
            float u1 = (float)(x + 1) / m_gridSize, v1 = (float)(z + 1) / m_gridSize;

            float r = 0.31f, g = 0.55f, b = 0.14f;

            Vertex vTL = { glm::vec4(x0, 0.0f, z0, 1.0f), r, g, b, {u0, v0}, {0, 1, 0} };
            Vertex vTR = { glm::vec4(x1, 0.0f, z0, 1.0f), r, g, b, {u1, v0}, {0, 1, 0} };
            Vertex vBL = { glm::vec4(x0, 0.0f, z1, 1.0f), r, g, b, {u0, v1}, {0, 1, 0} };
            Vertex vBR = { glm::vec4(x1, 0.0f, z1, 1.0f), r, g, b, {u1, v1}, {0, 1, 0} };

            m_terrainVertices.push_back(vTL);
            m_terrainVertices.push_back(vBL);
            m_terrainVertices.push_back(vTR);

            m_terrainVertices.push_back(vTR);
            m_terrainVertices.push_back(vBL);
            m_terrainVertices.push_back(vBR);
        }
    }

    // 8. POLICE HUD
    if (!m_fontTexture.LoadFromFile("external/Assets/boxy_bold_font_5.png")) {
        std::cerr << "[Erreur] Font non trouvee !\n";
    }

    // 9. CONFIGURATION DU FEU & DE LA FUMÉE
    glm::vec3 firePos = m_gameObjects[1].transform.translation + glm::vec3(0.0f, 0.2f, 0.0f);

    m_fireEmitter.Clear();
    m_fireEmitter.position = firePos;
    m_fireEmitter.spawnRate = 75.0f;
    m_fireEmitter.velocityMin = glm::vec3(-0.35f, 1.5f, -0.35f);
    m_fireEmitter.velocityMax = glm::vec3(0.35f, 3.0f, 0.35f);
    m_fireEmitter.gravity = glm::vec3(0.0f, 0.8f, 0.0f);
    m_fireEmitter.colorStart = glm::vec4(1.0f, 0.55f, 0.05f, 1.0f);
    m_fireEmitter.colorEnd = glm::vec4(0.8f, 0.05f, 0.0f, 0.0f);
    m_fireEmitter.lifeTimeMin = 0.6f;
    m_fireEmitter.lifeTimeMax = 1.2f;
    m_fireEmitter.sizeStart = 0.25f;
    m_fireEmitter.sizeEnd = 0.04f;
    m_fireEmitter.turbulence = 0.2f;
    m_fireEmitter.SetEmissionEnabled(true);

    m_smokeEmitter.Clear();
    m_smokeEmitter.position = firePos + glm::vec3(0.0f, 0.7f, 0.0f);
    m_smokeEmitter.spawnRate = 18.0f;
    m_smokeEmitter.velocityMin = glm::vec3(-0.35f, 0.4f, -0.35f);
    m_smokeEmitter.velocityMax = glm::vec3(0.35f, 0.9f, 0.35f);
    m_smokeEmitter.gravity = glm::vec3(0.0f, 0.12f, 0.0f);
    m_smokeEmitter.colorStart = glm::vec4(0.25f, 0.27f, 0.30f, 0.45f);
    m_smokeEmitter.colorEnd = glm::vec4(0.45f, 0.45f, 0.48f, 0.0f);
    m_smokeEmitter.lifeTimeMin = 2.5f;
    m_smokeEmitter.lifeTimeMax = 4.5f;
    m_smokeEmitter.sizeStart = 0.20f;
    m_smokeEmitter.sizeEnd = 1.1f;
    m_smokeEmitter.spawnRadius = 0.12f;
    m_smokeEmitter.drag = 0.25f;
    m_smokeEmitter.turbulence = 0.3f;
    m_smokeEmitter.SetEmissionEnabled(true);

}

// =====================================================
// UPDATE
// =====================================================
void LastScene::Update(float deltaTime)
{
    Scene::Update(deltaTime);
    m_time += deltaTime;

    if (!m_particlesFrozen)
    {
        m_hudAnimationTime += deltaTime;

        if (m_gameObjects.size() > 1 && m_gameObjects[1].loaded) {
            glm::vec3 firePos = m_gameObjects[1].transform.translation + glm::vec3(0.0f, 0.2f, 0.0f);
            m_fireEmitter.position = firePos;
            m_smokeEmitter.position = firePos + glm::vec3(0.0f, 0.7f, 0.0f);
        }

        m_fireEmitter.Update(deltaTime);
        m_smokeEmitter.Update(deltaTime);


    }
}

// =====================================================
// RENDER
// =====================================================
void LastScene::Render(GraphicsPipeline& pipeline, SceneManager& sceneManager, const glm::mat4& viewMatrix, const glm::mat4& viewProjection, const glm::vec3& viewPos, float deltaTime)
{
    glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, 1.0f, 0.5f));

    // -----------------------------------------------------
    // VUE FIXE ET PROJECTION ORTHOGONALE ISOMÉTRIQUE
    // -----------------------------------------------------
    glm::mat4 effectiveViewProj;
    glm::vec3 effectiveViewPos = viewPos;
    glm::mat4 effectiveView = viewMatrix;

    if (m_isEditMode) {
        effectiveViewProj = viewProjection;
    }
    else {
        float aspect = static_cast<float>(sceneManager.GetWidth()) / static_cast<float>(sceneManager.GetHeight());

        // Taille orthogonale resserrée pour cadrer précisément le village sur tout l'écran
        float orthoSize = 4.5f;
        glm::mat4 orthoProj = glm::ortho(-orthoSize * aspect, orthoSize * aspect, -orthoSize, orthoSize, 0.1f, 100.0f);

        // Vue isométrique plongeante fixe centrée sur le feu et la maison
        effectiveViewPos = glm::vec3(4.5f, 6.5f, -1.5f);
        glm::vec3 targetPos = glm::vec3(-0.5f, -1.2f, -6.0f);
        effectiveView = glm::lookAt(effectiveViewPos, targetPos, glm::vec3(0.0f, 1.0f, 0.0f));

        effectiveViewProj = orthoProj * effectiveView;
    }

    // -----------------------------------------------------
    // PASSE 1 : ARRIÈRE-PLAN (CIEL NDC) & RESET Z-BUFFER
    // -----------------------------------------------------
    pipeline.SetFragmentShader([](const BordLigne& pixel) {
        uint8_t ir = static_cast<uint8_t>(std::clamp(255.0f * pixel.r + 0.5f, 0.0f, 255.0f));
        uint8_t ig = static_cast<uint8_t>(std::clamp(255.0f * pixel.g + 0.5f, 0.0f, 255.0f));
        uint8_t ib = static_cast<uint8_t>(std::clamp(255.0f * pixel.b + 0.5f, 0.0f, 255.0f));
        return (0xFF << 24) | (ir << 16) | (ig << 8) | ib;
        });

    Vertex baseBackground[4] = {
        { { -1.0f,  1.0f, 0.9999f, 1.0f }, 0.2f, 0.4f, 0.8f },
        { { -1.0f, -1.0f, 0.9999f, 1.0f }, 0.6f, 0.8f, 1.0f },
        { {  1.0f, -1.0f, 0.9999f, 1.0f }, 0.6f, 0.8f, 1.0f },
        { {  1.0f,  1.0f, 0.9999f, 1.0f }, 0.2f, 0.4f, 0.8f }
    };
    pipeline.DrawSquareNDC(baseBackground[0], baseBackground[1], baseBackground[2], baseBackground[3]);
    pipeline.ClearZBuffer();
    pipeline.RenderScene();

    // -----------------------------------------------------
    // PASSE 2 : RENDU DES GAMEOBJECTS 3D
    // -----------------------------------------------------
    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        if (!m_gameObjects[i].loaded) continue;

        // On n'affiche pas la chouette (index 0) en vue isométrique globale
        if (i == 0) continue;

        if (i == 1) {
            // Index 1 : Feu de Camp Incandescent
            pipeline.SetFragmentShader([this](const BordLigne& pixel) -> uint32_t {
                float flicker = std::sin(m_time * 8.0f + pixel.worldPos.y * 10.0f) * 0.15f + 0.85f;
                float heightGrad = std::clamp((pixel.worldPos.y + 0.6f) / 0.8f, 0.0f, 1.0f);

                glm::vec3 colorRed(0.85f, 0.10f, 0.02f);
                glm::vec3 colorOrange(1.00f, 0.45f, 0.05f);
                glm::vec3 colorYellow(1.00f, 0.85f, 0.20f);

                glm::vec3 baseColor = glm::mix(colorRed, colorOrange, heightGrad);
                baseColor = glm::mix(baseColor, colorYellow, heightGrad * heightGrad) * flicker;

                uint8_t r = static_cast<uint8_t>(std::clamp(baseColor.r * 255.0f, 0.0f, 255.0f));
                uint8_t g = static_cast<uint8_t>(std::clamp(baseColor.g * 255.0f, 0.0f, 255.0f));
                uint8_t b = static_cast<uint8_t>(std::clamp(baseColor.b * 255.0f, 0.0f, 255.0f));

                return (0xFF << 24) | (r << 16) | (g << 8) | b;
                });

            m_gameObjects[i].UpdateTransform();
            glm::mat4 mvpObj = effectiveViewProj * m_gameObjects[i].modelMatrix;
            pipeline.DrawMesh(m_gameObjects[i].gpuVertices, mvpObj);
            pipeline.RenderScene();
        }
        else if (i == 2) {
            // Index 2 : MAISON (Vertex Shader ondulant + Fragment Shader arc-en-ciel)
            pipeline.SetVertexShader([this](const Vertex& vertex, const glm::mat4& mvp) {
                Vertex transformed = vertex;

                float waveX = std::sin(vertex.position.y * 8.0f + m_time * 5.0f) * 0.15f;
                float waveZ = std::cos(vertex.position.x * 6.0f + m_time * 4.0f) * 0.10f;
                float stretchY = std::sin(m_time * 3.0f) * 0.08f;

                transformed.position.x += waveX;
                transformed.position.z += waveZ;
                transformed.position.y += stretchY * vertex.position.y;

                transformed.position = mvp * transformed.position;
                glm::mat3 normalMatrix = glm::mat3(mvp);
                transformed.normal = glm::normalize(normalMatrix * vertex.normal);

                return transformed;
                });

            pipeline.SetFragmentShader([this, i, lightDir, effectiveViewPos](const BordLigne& pixel) -> uint32_t {
                const Material& mat = m_gameObjects[i].objModel.material;
                glm::vec3 baseDiffuse = mat.diffuse;

                float rRainbow = 0.5f + 0.5f * std::sin(m_time * 3.0f + pixel.worldPos.x * 2.0f);
                float gRainbow = 0.5f + 0.5f * std::sin(m_time * 3.0f + pixel.worldPos.y * 2.0f + 2.0f);
                float bRainbow = 0.5f + 0.5f * std::sin(m_time * 3.0f + pixel.worldPos.z * 2.0f + 4.0f);
                glm::vec3 rainbowColor(rRainbow, gRainbow, bRainbow);

                if (mat.hasTexture) {
                    uint32_t texColor = mat.texture.Sample(pixel.u, pixel.v);
                    float rTex = ((texColor >> 16) & 0xFF) / 255.0f;
                    float gTex = ((texColor >> 8) & 0xFF) / 255.0f;
                    float bTex = (texColor & 0xFF) / 255.0f;
                    baseDiffuse *= glm::vec3(rTex, gTex, bTex);
                }

                baseDiffuse = glm::mix(baseDiffuse, rainbowColor * 1.3f, 0.65f);

                glm::vec3 normal = glm::normalize(glm::vec3(pixel.nx, pixel.ny, pixel.nz));
                glm::vec3 pixelPos = glm::vec3(pixel.worldPos.x, pixel.worldPos.y, pixel.worldPos.z);
                glm::vec3 vV = glm::normalize(effectiveViewPos - pixelPos);
                glm::vec3 vH = glm::normalize(lightDir + vV);

                glm::vec3 ambient = mat.ambient * baseDiffuse;
                float diffFactor = std::max(0.0f, glm::dot(normal, lightDir));
                glm::vec3 diffuse = baseDiffuse * diffFactor;
                float specFactor = std::pow(std::max(0.0f, glm::dot(normal, vH)), mat.shininess);
                glm::vec3 specular = mat.specular * specFactor;

                glm::vec3 finalColor = ambient + diffuse + specular;
                uint8_t r = static_cast<uint8_t>(std::clamp(finalColor.r * 255.0f + 0.5f, 0.0f, 255.0f));
                uint8_t g = static_cast<uint8_t>(std::clamp(finalColor.g * 255.0f + 0.5f, 0.0f, 255.0f));
                uint8_t b = static_cast<uint8_t>(std::clamp(finalColor.b * 255.0f + 0.5f, 0.0f, 255.0f));

                return (0xFF << 24) | (r << 16) | (g << 8) | b;
                });

            m_gameObjects[i].UpdateTransform();
            glm::mat4 mvpObj = effectiveViewProj * m_gameObjects[i].modelMatrix;
            pipeline.DrawMesh(m_gameObjects[i].gpuVertices, mvpObj);
            pipeline.RenderScene();

            pipeline.SetVertexShader([](const Vertex& vertex, const glm::mat4& mvp) {
                Vertex transformed = vertex;
                transformed.position = mvp * vertex.position;
                glm::mat3 normalMatrix = glm::mat3(mvp);
                transformed.normal = glm::normalize(normalMatrix * vertex.normal);
                return transformed;
                });
        }
        else {
            // Index > 2 : Arbres, Ours, Loups
            pipeline.SetFragmentShader([this, i, lightDir, effectiveViewPos](const BordLigne& pixel) -> uint32_t {
                const Material& mat = m_gameObjects[i].objModel.material;
                glm::vec3 baseDiffuse = mat.diffuse;

                if (mat.hasTexture) {
                    uint32_t texColor = mat.texture.Sample(pixel.u, pixel.v);
                    float rTex = ((texColor >> 16) & 0xFF) / 255.0f;
                    float gTex = ((texColor >> 8) & 0xFF) / 255.0f;
                    float bTex = (texColor & 0xFF) / 255.0f;
                    baseDiffuse *= glm::vec3(rTex, gTex, bTex);
                }

                glm::vec3 normal = glm::normalize(glm::vec3(pixel.nx, pixel.ny, pixel.nz));
                glm::vec3 pixelPos = glm::vec3(pixel.worldPos.x, pixel.worldPos.y, pixel.worldPos.z);
                glm::vec3 vV = glm::normalize(effectiveViewPos - pixelPos);
                glm::vec3 vH = glm::normalize(lightDir + vV);

                glm::vec3 ambient = mat.ambient * baseDiffuse;
                float diffFactor = std::max(0.0f, glm::dot(normal, lightDir));
                glm::vec3 diffuse = baseDiffuse * diffFactor;
                float specFactor = std::pow(std::max(0.0f, glm::dot(normal, vH)), mat.shininess);
                glm::vec3 specular = mat.specular * specFactor;

                glm::vec3 finalColor = ambient + diffuse + specular;
                uint8_t r = static_cast<uint8_t>(std::clamp(finalColor.r * 255.0f + 0.5f, 0.0f, 255.0f));
                uint8_t g = static_cast<uint8_t>(std::clamp(finalColor.g * 255.0f + 0.5f, 0.0f, 255.0f));
                uint8_t b = static_cast<uint8_t>(std::clamp(finalColor.b * 255.0f + 0.5f, 0.0f, 255.0f));

                return (0xFF << 24) | (r << 16) | (g << 8) | b;
                });

            m_gameObjects[i].UpdateTransform();
            glm::mat4 mvpObj = effectiveViewProj * m_gameObjects[i].modelMatrix;
            pipeline.DrawMesh(m_gameObjects[i].gpuVertices, mvpObj);
            pipeline.RenderScene();
        }
    }

    // -----------------------------------------------------
    // PASSE 3 : TERRAIN (HERBE VERTE & COLLINES ADOUCIES)
    // -----------------------------------------------------
    pipeline.SetFragmentShader([lightDir, effectiveViewPos](const BordLigne& pixel) -> uint32_t {
        glm::vec3 baseColor(pixel.r, pixel.g, pixel.b);
        glm::vec3 normal = glm::normalize(glm::vec3(pixel.nx, pixel.ny, pixel.nz));
        glm::vec3 pixelPos = glm::vec3(pixel.worldPos.x, pixel.worldPos.y, pixel.worldPos.z);
        glm::vec3 vV = glm::normalize(effectiveViewPos - pixelPos);
        glm::vec3 vH = glm::normalize(lightDir + vV);

        glm::vec3 ambient = baseColor * 0.35f;
        float diffFactor = std::max(0.0f, glm::dot(normal, lightDir));
        glm::vec3 diffuse = baseColor * diffFactor * 0.75f;

        float shininess = 128.0f;
        float specFactor = std::pow(std::max(0.0f, glm::dot(normal, vH)), shininess);
        glm::vec3 specular = glm::vec3(0.15f) * specFactor;

        glm::vec3 finalColor = ambient + diffuse + specular;
        uint8_t r = static_cast<uint8_t>(std::clamp(finalColor.r * 255.0f + 0.5f, 0.0f, 255.0f));
        uint8_t g = static_cast<uint8_t>(std::clamp(finalColor.g * 255.0f + 0.5f, 0.0f, 255.0f));
        uint8_t b = static_cast<uint8_t>(std::clamp(finalColor.b * 255.0f + 0.5f, 0.0f, 255.0f));

        return (0xFF << 24) | (r << 16) | (g << 8) | b;
        });

    // Fixation de l'ancrage du terrain pour éviter tout décalage visuel
    glm::mat4 modelTerrain = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -1.6f, -6.0f));
    glm::mat4 mvpTerrain = effectiveViewProj * modelTerrain;

    pipeline.SetVertexShader([modelTerrain](const Vertex& vertex, const glm::mat4& mvp) {
        Vertex transformed = vertex;
        glm::vec4 worldPosCoord = modelTerrain * vertex.position;

        float hillScale = 0.3f;
        float height = SimpleNoise(worldPosCoord.x, worldPosCoord.z) * hillScale;
        transformed.position.y += height;

        float eps = 0.1f;
        float hL = SimpleNoise(worldPosCoord.x - eps, worldPosCoord.z) * hillScale;
        float hR = SimpleNoise(worldPosCoord.x + eps, worldPosCoord.z) * hillScale;
        float hD = SimpleNoise(worldPosCoord.x, worldPosCoord.z - eps) * hillScale;
        float hU = SimpleNoise(worldPosCoord.x, worldPosCoord.z + eps) * hillScale;

        glm::vec3 tangentX(2.0f * eps, hR - hL, 0.0f);
        glm::vec3 tangentZ(0.0f, hU - hD, 2.0f * eps);
        glm::vec3 surfaceNormal = glm::normalize(glm::cross(tangentZ, tangentX));

        transformed.position = mvp * transformed.position;
        glm::mat3 normalMatrix = glm::mat3(mvp);
        transformed.normal = glm::normalize(normalMatrix * surfaceNormal);

        return transformed;
        });

    pipeline.DrawMesh(m_terrainVertices, mvpTerrain);
    pipeline.RenderScene();

    // Reset du Vertex Shader standard
    pipeline.SetVertexShader([](const Vertex& vertex, const glm::mat4& mvp) {
        Vertex transformed = vertex;
        transformed.position = mvp * vertex.position;
        glm::mat3 normalMatrix = glm::mat3(mvp);
        transformed.normal = glm::normalize(normalMatrix * vertex.normal);
        return transformed;
        });

    // -----------------------------------------------------
    // PASSE 4 : PARTICULES (FEU, FUMÉE, EAU)
    // -----------------------------------------------------
    pipeline.SetFragmentShader([](const BordLigne& pixel) -> uint32_t {
        float dx = (pixel.u - 0.5f) * 2.0f;
        float dy = (pixel.v - 0.5f) * 2.0f;
        float radius = std::sqrt(dx * dx + dy * dy);

        float t = std::clamp((radius - 0.35f) / 0.65f, 0.0f, 1.0f);
        float smooth = t * t * (3.0f - 2.0f * t);
        float edgeAlpha = 1.0f - smooth;
        float finalAlpha = pixel.alpha * edgeAlpha;

        uint8_t a = static_cast<uint8_t>(std::clamp(finalAlpha * 255.0f, 0.0f, 255.0f));
        uint8_t r = static_cast<uint8_t>(std::clamp(pixel.r * 255.0f + 0.5f, 0.0f, 255.0f));
        uint8_t g = static_cast<uint8_t>(std::clamp(pixel.g * 255.0f + 0.5f, 0.0f, 255.0f));
        uint8_t b = static_cast<uint8_t>(std::clamp(pixel.b * 255.0f + 0.5f, 0.0f, 255.0f));

        return (a << 24) | (r << 16) | (g << 8) | b;
        });

    pipeline.SetAlphaBlendingEnabled(true);
    pipeline.SetDepthWriteEnabled(false);

    m_fireEmitter.Render(pipeline, effectiveView, effectiveViewProj);
    m_smokeEmitter.Render(pipeline, effectiveView, effectiveViewProj);

    pipeline.RenderScene();

    pipeline.SetDepthWriteEnabled(true);
    pipeline.SetAlphaBlendingEnabled(false);
}

// =====================================================
// CLEANUP & EVENTS
// =====================================================
void LastScene::OnExit()
{
    m_gameObjects.clear();
    std::cout << "Scene LastScene quittee\n";
}

void LastScene::SetParticlesFrozen(bool frozen)
{
    m_particlesFrozen = frozen;
}

void LastScene::SetEditMode(bool editMode)
{
    m_isEditMode = editMode;
    SetParticlesFrozen(editMode);
}