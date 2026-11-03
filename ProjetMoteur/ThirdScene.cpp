#include "ThirdScene.h"
#include "MainScene.h"
#include "GraphicsPipeline.h"
#include "Camera.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "MaterialPresets.h"

ThirdScene::ThirdScene(SceneManager& sceneManager) : Scene(sceneManager)
{
    //m_keys.push_back({ &m_objModel.translation.x, {

    //} });
}

ThirdScene::ThirdScene(SceneManager& sceneManager, int gridSize, float cellSize) :  Scene(sceneManager)
{
    m_gridSize = gridSize;
    m_cellSize = cellSize;
}

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

        // Si la texture n'a pas été chargée automatiquement via le .mtl, on la charge manuellement :
        if (!m_objModel.material.hasTexture) {
            if (m_objModel.material.texture.LoadFromFile(texturePath)) {
                m_objModel.material.hasTexture = true;
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

   m_objModel.material.diffuse = glm::vec3(1.0f, 1.0f, 1.0f); 


   m_gpuVertices.clear();
   m_gpuVertices.reserve(m_objModel.vertices.size());

   for (const auto& src : m_objModel.vertices) {
       m_gpuVertices.push_back({
           src.position, src.r, src.g, src.b,
           {src.tx, src.ty}, {src.nx, src.ny, src.nz}
           });
   }


   // TESTE MATERIAL
   // On applique un matériau Or au modèle
   Texture oldTex = m_objModel.material.texture; // Conserver la texture si besoin
   bool hadTex = m_objModel.material.hasTexture;

   m_objModel.material = MaterialPresets::Gold();
   m_objModel.material.texture = oldTex;
   m_objModel.material.hasTexture = hadTex;


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

           // Couleur bleue de base pour l'eau
           float r = 0.1f, g = 0.4f, b = 0.8f;

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


   if (m_fontTexture.LoadFromFile("external/Assets/boxy_bold_font_5.png"))
       std::cout << "font texte charge !\n";
   else
       std::cout << "font texte pas trouve !\n";

}


void ThirdScene::Update(float deltaTime)
{
    m_time += deltaTime;

    m_hudAnimationTime += deltaTime;

    // TESTE MATERIAL 
    if (m_loaded) {
        // 1. Oscillation de la brillance de 4.0 (mat) à 128.0 (miroir)
        m_objModel.material.shininess = 66.0f + std::sin(m_time * 2.0f) * 62.0f;

        // 2. Pulsation de la couleur du reflet spéculaire (Blanc -> Magenta)
        float pulse = (std::sin(m_time * 3.0f) + 1.0f) * 0.5f; // [0.0, 1.0]
        m_objModel.material.specular = glm::vec3(1.0f, 1.0f - pulse, 1.0f);
    }
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

    //// 2. SOL (En NDC)
    //Vertex basePlan[4] = {
    //    { { -1.0f,  0.0f, 0.999f, 1.0f }, 0.4f, 0.4f, 0.4f },
    //    { { -1.0f, -1.0f, 0.999f, 1.0f }, 0.2f, 0.2f, 0.2f },
    //    { {  1.0f, -1.0f, 0.999f, 1.0f }, 0.2f, 0.2f, 0.2f },
    //    { {  1.0f,  0.0f, 0.999f, 1.0f }, 0.4f, 0.4f, 0.4f }
    //};
    //pipeline.DrawSquareNDC(basePlan[0], basePlan[1], basePlan[2], basePlan[3]);

    pipeline.ClearZBuffer();

    pipeline.RenderScene();










    // 3. RENDU DU MODÈLE OBJ CHARGÉ
    if (m_loaded) {

        pipeline.SetFragmentShader([this, lightDir, viewPos](const BordLigne& pixel) -> uint32_t {
            const Material& mat = m_objModel.material;

            // 1. On récupère la couleur interpolée du sommet (r, g, b)
            glm::vec3 vertexColor(pixel.r, pixel.g, pixel.b);

            // 2. Couleur diffuse de base = Matériau (Kd) x Couleur du sommet
            glm::vec3 baseDiffuse = mat.diffuse * vertexColor;

            // 3. Si une texture existe, elle vient teinter / moduler la couleur de base
            if (mat.hasTexture) {
                uint32_t texColor = mat.texture.Sample(pixel.u, pixel.v);
                float rTex = ((texColor >> 16) & 0xFF) / 255.0f;
                float gTex = ((texColor >> 8) & 0xFF) / 255.0f;
                float bTex = (texColor & 0xFF) / 255.0f;

                baseDiffuse *= glm::vec3(rTex, gTex, bTex);
            }

            // --- Calcul Blinn-Phong ---
            glm::vec3 normal = glm::normalize(glm::vec3(pixel.nx, pixel.ny, pixel.nz));
            glm::vec3 pixelPos = glm::vec3(pixel.worldPos.x, pixel.worldPos.y, pixel.worldPos.z);
            glm::vec3 vV = glm::normalize(viewPos - pixelPos);
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

        glm::mat4 modelOBJ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.2f, 0.7f));
        modelOBJ = glm::scale(modelOBJ, glm::vec3(0.05f, 0.05f, 0.05f));
        modelOBJ = glm::rotate(modelOBJ, deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 mvpOBJ = viewProjection * modelOBJ;

        //for (size_t i = 0; i + 2 < m_objModel.vertices.size(); i += 3) {
        //    auto& src0 = m_objModel.vertices[i + 0];
        //    auto& src1 = m_objModel.vertices[i + 1];
        //    auto& src2 = m_objModel.vertices[i + 2];

        //    Vertex v0 = { src0.position, src0.r, src0.g, src0.b, {src0.tx, src0.ty}, {src0.nx, src0.ny, src0.nz} };
        //    Vertex v1 = { src1.position, src1.r, src1.g, src1.b, {src1.tx, src1.ty}, {src1.nx, src1.ny, src1.nz} };
        //    Vertex v2 = { src2.position, src2.r, src2.g, src2.b, {src2.tx, src2.ty}, {src2.nx, src2.ny, src2.nz} };

        //    pipeline.DrawTriangle(v0, v1, v2, mvpOBJ);
        //}

        pipeline.DrawMesh(m_gpuVertices, mvpOBJ);

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

    // Reset du Vertex Shader pour les prochains objets de la frame suivante
    pipeline.SetVertexShader([](const Vertex& vertex, const glm::mat4& mvp) {
        Vertex transformed = vertex;
        transformed.position = mvp * vertex.position;
        glm::mat3 normalMatrix = glm::mat3(mvp);
        transformed.normal = glm::normalize(normalMatrix * vertex.normal);
        return transformed;
        });

    // --- SHADER DE L'EAU ---

// 1. FRAGMENT SHADER (Lumière Blinn-Phong pour l'eau)
    pipeline.SetFragmentShader([lightDir, viewPos](const BordLigne& pixel) -> uint32_t {
        // Couleur de base (le bleu qu'on a mis dans GenerateTerrain)
        glm::vec3 baseColor(pixel.r, pixel.g, pixel.b);

        glm::vec3 normal = glm::normalize(glm::vec3(pixel.nx, pixel.ny, pixel.nz));
        glm::vec3 pixelPos = glm::vec3(pixel.worldPos.x, pixel.worldPos.y, pixel.worldPos.z);

        glm::vec3 vV = glm::normalize(viewPos - pixelPos);
        glm::vec3 vH = glm::normalize(lightDir + vV);

        // Lumière Ambiante et Diffuse
        glm::vec3 ambient = baseColor * 0.3f;
        float diffFactor = std::max(0.0f, glm::dot(normal, lightDir));
        glm::vec3 diffuse = baseColor * diffFactor * 0.8f;

        // Lumière Spéculaire (Le reflet du soleil sur les vagues !)
        float shininess = 128.0f; // Très élevé pour un reflet net typique de l'eau
        float specFactor = std::pow(std::max(0.0f, glm::dot(normal, vH)), shininess);
        glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f) * specFactor; // Reflet blanc pur

        glm::vec3 finalColor = ambient + diffuse + specular;

        // Conversion en pixels
        uint8_t r = static_cast<uint8_t>(std::clamp(finalColor.r * 255.0f + 0.5f, 0.0f, 255.0f));
        uint8_t g = static_cast<uint8_t>(std::clamp(finalColor.g * 255.0f + 0.5f, 0.0f, 255.0f));
        uint8_t b = static_cast<uint8_t>(std::clamp(finalColor.b * 255.0f + 0.5f, 0.0f, 255.0f));

        return (0xFF << 24) | (r << 16) | (g << 8) | b;
        });


    // 1. Récupération de la position de la caméra pour ancrer la grille
    // (Supposons que viewPos contienne la position 3D de la caméra)
    float snapSpacing = m_cellSize;
    float snappedCamX = std::floor(viewPos.x / snapSpacing) * snapSpacing;
    float snappedCamZ = std::floor(viewPos.z / snapSpacing) * snapSpacing;
    //float snappedCamX = viewPos.x;
    //float snappedCamZ = viewPos.z;

    // Matrice modèle du terrain centrée sous la caméra
    glm::mat4 modelTerrain = glm::translate(glm::mat4(1.0f), glm::vec3(snappedCamX, -1.0f, snappedCamZ));
    glm::mat4 mvpTerrain = viewProjection * modelTerrain;


    pipeline.SetVertexShader([deltaTime, modelTerrain](const Vertex& vertex, const glm::mat4& mvp) {
        Vertex transformed = vertex;

        // On calcule la position absolue dans le monde pour que le bruit/vagues 
        // ne bouge pas de manière rigide avec la grille, mais reste fixé dans l'espace global.
        glm::vec4 worldPosCoord = modelTerrain * vertex.position;

        float amplitude = 0.4f;
        float frequence = 1.5f;
        float vitesse = 2.5f;

        // Ondulations basées sur la position absolue du monde
        float waveX = std::sin(worldPosCoord.x * frequence + deltaTime * vitesse);
        float waveZ = std::cos(worldPosCoord.z * frequence + deltaTime * vitesse);

        transformed.position.y += (waveX + waveZ) * amplitude;

        // Recalcul mathématique de la normale pour la lumière spéculaire
        float dYdX = std::cos(worldPosCoord.x * frequence + deltaTime * vitesse) * frequence * amplitude;
        float dYdZ = -std::sin(worldPosCoord.z * frequence + deltaTime * vitesse) * frequence * amplitude;
        glm::vec3 surfaceNormal = glm::normalize(glm::vec3(-dYdX, 1.0f, -dYdZ));

        // Transformation standard MVP
        transformed.position = mvp * transformed.position;
        glm::mat3 normalMatrix = glm::mat3(mvp);
        transformed.normal = glm::normalize(normalMatrix * surfaceNormal);

        return transformed;
        });

    // Envoi au pipeline !
    pipeline.DrawMesh(m_terrainVertices, mvpTerrain);

    pipeline.RenderScene();

    // Reset du Vertex Shader pour les prochains objets de la frame suivante
    pipeline.SetVertexShader([](const Vertex& vertex, const glm::mat4& mvp) {
        Vertex transformed = vertex;
        transformed.position = mvp * vertex.position;
        glm::mat3 normalMatrix = glm::mat3(mvp);
        transformed.normal = glm::normalize(normalMatrix * vertex.normal);
        return transformed;
        });











    pipeline.DrawText2D(
        20, 20,
        "FPS: 320",
        m_fontTexture,
        0xFFFFFFFF
    );

    // =====================================================
    // ANIMATION DU TEXTE
    // =====================================================

// Durées des différentes phases.
    constexpr float fadeInDuration = 0.8f;
    constexpr float holdDuration = 1.4f;
    constexpr float fadeOutDuration = 0.8f;
    constexpr float pauseDuration = 1.0f;

    constexpr float cycleDuration =
        fadeInDuration + holdDuration +
        fadeOutDuration + pauseDuration;

    // Temps dans le cycle actuel.
    float animationTime = std::fmod(
        m_hudAnimationTime,
        cycleDuration
    );

    float alpha = 0.0f;
    float textX = 20.0f;

    // Fonction d'interpolation douce.
    auto SmoothStep = [](float t)
        {
            t = std::clamp(t, 0.0f, 1.0f);
            return t * t * (3.0f - 2.0f * t);
        };

    // PHASE 1 : APPARITION
    if (animationTime < fadeInDuration)
    {
        float progress = animationTime / fadeInDuration;
        float eased = SmoothStep(progress);

        alpha = eased;

        // Le texte arrive depuis la gauche.
        textX = -60.0f + 80.0f * eased;
    }

    // PHASE 2 : AFFICHAGE
    else if (animationTime < fadeInDuration + holdDuration)
    {
        alpha = 1.0f;
        textX = 20.0f;
    }

    // PHASE 3 : DISPARITION
    else if (animationTime <
        fadeInDuration + holdDuration + fadeOutDuration)
    {
        float progress =
            (animationTime - fadeInDuration - holdDuration)
            / fadeOutDuration;

        float eased = SmoothStep(progress);

        alpha = 1.0f - eased;

        // Le texte repart vers la droite.
        textX = 20.0f + 80.0f * eased;
    }

    // PHASE 4 : INVISIBLE
    else
    {
        alpha = 0.0f;
    }

    // Conversion de l'alpha en entier [0, 255].
    uint32_t alphaValue = static_cast<uint32_t>(
        std::round(alpha * 255.0f)
        );

    // Couleur Cyan avec transparence variable.
    uint32_t textColor =
        (alphaValue << 24) | 0x0000FFFF;

    // Affichage du texte animé.
    pipeline.DrawText2D(
        static_cast<int>(textX),
        45,
        "MOTEUR 3D CPU",
        m_fontTexture,
        textColor
    );
}

void ThirdScene::OnExit()
{
    m_objModel.vertices.clear();
    std::cout << "Troisieme Scene quittee\n";
}