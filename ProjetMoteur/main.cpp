#include <iostream>
#include <cmath>
#include <algorithm>
#include "MathUtils.h"
#include "ObjLoader.h"
#include "Window.h"
#include "GraphicsPipeline.h"

const int RENDER_WIDTH = 320;
const int RENDER_HEIGHT = 200;

int main(int argc, char* argv[])
{
    Window window(RENDER_WIDTH, RENDER_HEIGHT, 800, 500, "Moteur 3D CPU - Titre");
    if (!window.Init()) return 1;

    GraphicsPipeline pipeline(RENDER_WIDTH, RENDER_HEIGHT);

    // Chargement Obj
    T_OBJ objModel;
    const char* objPath = "external/Assets/catn0.obj";
    Get_TOBJ(objPath, objModel);
    bool loaded = !objModel.vertices.empty();

    if (loaded) {
        std::cout << "Obj Charge depuis : " << objPath << " (" << objModel.vertices.size() << " sommets)" << std::endl;
    }
    else {
        std::cerr << "Erreur : Impossible de charger " << objPath << std::endl;
    }

    bool isPerspective = true;
    float aspect = (float)RENDER_WIDTH / (float)RENDER_HEIGHT;
    float zNear = 0.1f;
    float zFar = 100.0f;




    
    


    while (window.PollEvents(isPerspective)) {
        Uint64 startPerf = SDL_GetPerformanceCounter();
        pipeline.ResetStats();
        pipeline.Clear();

        glm::mat4 projection;
        if (isPerspective) {
            projection = glm::perspective(glm::radians(60.0f), aspect, zNear, zFar);
        }
        else {
            float taille = 2.0f;
            projection = glm::ortho(-taille * aspect, taille * aspect, -taille, taille, zNear, zFar);
        }

        

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
            { { -1.0f,  1.0f, 0.9999f, 1.0f }, 0.2f, 0.4f, 0.8f },
            { { -1.0f, -1.0f, 0.9999f, 1.0f }, 0.6f, 0.8f, 1.0f },
            { {  1.0f, -1.0f, 0.9999f, 1.0f }, 0.6f, 0.8f, 1.0f },
            { {  1.0f,  1.0f, 0.9999f, 1.0f }, 0.2f, 0.4f, 0.8f }
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


        glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, 1.0f, 0.5f));
        glm::vec3 viewPos = glm::vec3(0.0f, 0.0f, 0.0f); // Position de la caméra
        float smoothness = 32.0f; // La brillance (plus c'est haut, plus le reflet est serré/net)

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

        //// LIGHT
        //glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, 1.0f, 0.5f));

        //// 2. On injecte ce vecteur 'lightDir' dans le Fragment Shader via [lightDir]
        //pipeline.SetFragmentShader([lightDir](const BordLigne& pixel) {

        //    // Récupération et normalisation de la normale du pixel
        //    glm::vec3 normal = glm::normalize(glm::vec3(pixel.nx, pixel.ny, pixel.nz));

        //    float diffuse = std::max(0.0f, glm::dot(normal, lightDir));

        //    float ambient = 0.5f;
        //    float lightIntensity = std::min(1.0f, ambient + diffuse);

        //    uint8_t r = static_cast<uint8_t>(std::clamp(255.0f * pixel.r * lightIntensity + 0.5f, 0.0f, 255.0f));
        //    uint8_t g = static_cast<uint8_t>(std::clamp(255.0f * pixel.g * lightIntensity + 0.5f, 0.0f, 255.0f));
        //    uint8_t b = static_cast<uint8_t>(std::clamp(255.0f * pixel.b * lightIntensity + 0.5f, 0.0f, 255.0f));

        //    return (0xFF << 24) | (r << 16) | (g << 8) | b;
        //    });





        pipeline.SetVertexShader([](const Vertex& vertex, const glm::mat4& mvp) {
            Vertex modified = vertex;

            // 1. Position pour l'écran (MVP)
            modified.position = mvp * vertex.position;

            // 2. Position monde approchée ou stockée (on peut utiliser la position locale ou la position transformée)
            modified.worldPos.x = vertex.position.x;
            modified.worldPos.y = vertex.position.y;
            modified.worldPos.z = vertex.position.z;

            // 3. Rotation de la normale
            glm::mat3 normalMatrix = glm::mat3(mvp);
            modified.normal = glm::normalize(normalMatrix * vertex.normal);

            return modified;
            });



        float time = SDL_GetTicks() * 0.001f;

        // 3. RENDU DU MODÈLE OBJ CHARGÉ
        if (loaded) {
            glm::mat4 modelOBJ = glm::mat4(1.0f);
            modelOBJ = glm::scale(modelOBJ, glm::vec3(0.05f, 0.05f, 0.05f));
            modelOBJ = glm::translate(modelOBJ, glm::vec3(0.0f, -3.0f, -20.0f));
            modelOBJ = glm::rotate(modelOBJ, time, glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat4 mvpOBJ = projection * modelOBJ;

            for (size_t i = 0; i + 2 < objModel.vertices.size(); i += 3) {
                auto& src0 = objModel.vertices[i + 0];
                auto& src1 = objModel.vertices[i + 1];
                auto& src2 = objModel.vertices[i + 2];

                Vertex v0 = { src0.position, src0.r, src0.g, src0.b, {src0.tx, src0.ty}, {src0.nx, src0.ny, src0.nz} };
                Vertex v1 = { src1.position, src1.r, src1.g, src1.b, {src1.tx, src1.ty}, {src1.nx, src1.ny, src1.nz} };
                Vertex v2 = { src2.position, src2.r, src2.g, src2.b, {src2.tx, src2.ty}, {src2.nx, src2.ny, src2.nz} };

                pipeline.DrawTriangle(v0, v1, v2, mvpOBJ);
            }
        }

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
        model1 = glm::translate(model1, glm::vec3(-3.0f, 0.0f, -4.5f));
        model1 = glm::rotate(model1, time, glm::vec3(1.0f, 0.7f, 0.0f));
        glm::mat4 mvp1 = projection * model1;

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



        pipeline.SetFragmentShader([](const BordLigne& pixel) {
            float time = SDL_GetTicks() * 0.003f;

            // On combine x et z (profondeur) avec le temps pour faire une onde
            float wave = std::sin(pixel.x * 0.05f + pixel.z * 15.0f + time);
            float intensity = (wave * 0.5f) + 0.5f; // Ramène entre 0.0 et 1.0

            // Dégradé Cyan / Violet cyberpunk
            uint8_t r = static_cast<uint8_t>(intensity * 255.0f);
            uint8_t g = static_cast<uint8_t>((1.0f - intensity) * 150.0f);
            uint8_t b = 255;

            return (0xFF << 24) | (r << 16) | (g << 8) | b;
            });

        // On personnalise le Vertex Shader pour faire onduler les sommets
        pipeline.SetVertexShader([](const Vertex& vertex, const glm::mat4& mvp) {
            Vertex modified = vertex;

            // Effet de vague sur l'axe Y basé sur le temps et la position X
            float time = SDL_GetTicks() * 0.003f;
            modified.position.y += std::sin(time + modified.position.x * 3.0f) * 0.1f;

            // Application classique de la matrice MVP
            modified.position = mvp * modified.position;
            return modified;
            });

        // 5. CUBE 3D 2
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(1.0f, 0.0f, -3.5f));
        model2 = glm::rotate(model2, time * 2.f, glm::vec3(-1.0f, 0.7f, 0.0f));
        glm::mat4 mvp2 = projection * model2;

        Cube cubeFinal2 = {
            baseCube[0], baseCube[1], baseCube[2], baseCube[3],
            baseCube[7], baseCube[6], baseCube[5], baseCube[4],
            baseCube[4], baseCube[5], baseCube[1], baseCube[0],
            baseCube[3], baseCube[2], baseCube[6], baseCube[7],
            baseCube[4], baseCube[0], baseCube[3], baseCube[7],
            baseCube[1], baseCube[5], baseCube[6], baseCube[2]
        };
        pipeline.DrawCube(cubeFinal2, mvp2);
        


        // 6. TRIANGLE 3D SUPPLÉMENTAIRE
        Vertex verticesTri[3] = {
            { {  0.0f,  0.4f, 0.0f, 1.0f}, 1.0f, 0.0f, 0.0f },
            { { -0.4f, -0.4f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f },
            { {  0.4f, -0.4f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f }
        };
        glm::mat4 model3 = glm::mat4(1.0f);
        model3 = glm::translate(model3, glm::vec3(-1.0f, 0.0f, -1.0f));
        model3 = glm::rotate(model3, time * 1.5f, glm::vec3(-1.0f, 0.0f, 0.0f));
        glm::mat4 mvp3 = projection * model3;

        pipeline.DrawTriangle(verticesTri[0], verticesTri[1], verticesTri[2], mvp3);

        // Lancement des threads de calcul du rastériseur
        pipeline.RenderScene();




        // Envoi à la fenêtre SDL
        window.UpdateTexture(pipeline.GetPixels(), pipeline.GetPitch());
        window.Render();





        // Statistiques de performance
        Uint64 endPerf = SDL_GetPerformanceCounter();
        float frameTimeMs = (endPerf - startPerf) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
        const RenderStats& stats = pipeline.GetStats();

        static Uint32 lastLogTime = 0;
        static int frameCount = 0;
        Uint32 currentTime = SDL_GetTicks();
        frameCount++;

        if (currentTime - lastLogTime >= 1000) {
            printf("FPS: %d | CPU: %.2f ms | Triangles traites: %d | Triangles rasterises: %d | Pixels: %d | Threads: %d\n",
                frameCount,
                frameTimeMs,
                stats.trianglesProcessed,
                stats.trianglesRasterized,
                stats.pixelsProcessed.load(),
                stats.threadsUsed);

            lastLogTime = currentTime;
            frameCount = 0;
        }
    }
    return 0;
}