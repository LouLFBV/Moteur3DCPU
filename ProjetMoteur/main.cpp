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
    const char* objPath = "external/Assets/base.obj";
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






    //// On personnalise le Vertex Shader pour faire onduler les sommets
    //pipeline.SetVertexShader([](const Vertex& vertex, const glm::mat4& mvp) {
    //    Vertex modified = vertex;

    //    // Effet de vague sur l'axe Y basé sur le temps et la position X
    //    float time = SDL_GetTicks() * 0.003f;
    //    modified.position.y += std::sin(time + modified.position.x * 3.0f) * 0.1f;

    //    // Application classique de la matrice MVP
    //    modified.position = mvp * modified.position;
    //    return modified;
    //    });

    //pipeline.SetFragmentShader([](const BordLigne& pixel) {
    //    // pixel.z varie généralement entre 0 (proche) et 1 (loin)
    //    float depth = std::clamp(pixel.z, 0.0f, 1.0f);

    //    // On inverse pour que le lointain soit noir et le proche soit lumineux
    //    float intensity = 1.0f - depth;

    //    uint8_t c = static_cast<uint8_t>(intensity * 255.0f);
    //    return (0xFF << 24) | (c << 16) | (c << 8) | c; // Niveaux de gris selon la profondeur
    //    });



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

        // 1. FOND (Ciel en NDC)
        Vertex baseBackground[4] = {
            { { -1.0f,  1.0f, 0.99f, 1.0f }, 0.2f, 0.4f, 0.8f },
            { { -1.0f, -1.0f, 0.99f, 1.0f }, 0.6f, 0.8f, 1.0f },
            { {  1.0f, -1.0f, 0.99f, 1.0f }, 0.6f, 0.8f, 1.0f },
            { {  1.0f,  1.0f, 0.99f, 1.0f }, 0.2f, 0.4f, 0.8f }
        };
        pipeline.DrawSquareNDC(baseBackground[0], baseBackground[1], baseBackground[2], baseBackground[3]);

        // 2. SOL (En NDC)
        Vertex basePlan[4] = {
            { { -1.0f,  0.0f, 0.95f, 1.0f }, 0.4f, 0.4f, 0.4f },
            { { -1.0f, -1.0f, 0.95f, 1.0f }, 0.2f, 0.2f, 0.2f },
            { {  1.0f, -1.0f, 0.95f, 1.0f }, 0.2f, 0.2f, 0.2f },
            { {  1.0f,  0.0f, 0.95f, 1.0f }, 0.4f, 0.4f, 0.4f }
        };
        pipeline.DrawSquareNDC(basePlan[0], basePlan[1], basePlan[2], basePlan[3]);

        pipeline.ClearZBuffer();

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


        float time = SDL_GetTicks() * 0.001f;

        // 3. RENDU DU MODÈLE OBJ CHARGÉ
        if (loaded) {
            glm::mat4 modelOBJ = glm::mat4(1.0f);
            modelOBJ = glm::translate(modelOBJ, glm::vec3(-2.0f, 0.0f, -3.0f));
            modelOBJ = glm::rotate(modelOBJ, time, glm::vec3(0.0f, 1.0f, 1.0f));

            glm::mat4 mvpOBJ = projection * modelOBJ;

            for (size_t i = 0; i + 2 < objModel.vertices.size(); i += 3) {
                Vertex v0 = { objModel.vertices[i + 0].position, objModel.vertices[i + 0].r, objModel.vertices[i + 0].g, objModel.vertices[i + 0].b };
                Vertex v1 = { objModel.vertices[i + 1].position, objModel.vertices[i + 1].r, objModel.vertices[i + 1].g, objModel.vertices[i + 1].b };
                Vertex v2 = { objModel.vertices[i + 2].position, objModel.vertices[i + 2].r, objModel.vertices[i + 2].g, objModel.vertices[i + 2].b };

                // Le pipeline gère le vertex shader et le dessin
                pipeline.DrawTriangle(v0, v1, v2, mvpOBJ);
            }
        }

        // Données des cubes
        Vertex baseCube[8] = {
            { { -0.5f,  0.5f,  0.5f, 1.0f }, 1.0f, 0.0f, 0.0f },
            { { -0.5f, -0.5f,  0.5f, 1.0f }, 0.0f, 1.0f, 0.0f },
            { {  0.5f, -0.5f,  0.5f, 1.0f }, 0.0f, 0.0f, 1.0f },
            { {  0.5f,  0.5f,  0.5f, 1.0f }, 1.0f, 1.0f, 0.0f },
            { { -0.5f,  0.5f, -0.5f, 1.0f }, 0.0f, 1.0f, 1.0f },
            { { -0.5f, -0.5f, -0.5f, 1.0f }, 1.0f, 0.0f, 1.0f },
            { {  0.5f, -0.5f, -0.5f, 1.0f }, 1.0f, 1.0f, 1.0f },
            { {  0.5f,  0.5f, -0.5f, 1.0f }, 0.5f, 0.5f, 0.5f }
        };


        //pipeline.SetFragmentShader([](const BordLigne& pixel) {
        //    float time = SDL_GetTicks() * 0.003f;

        //    // On combine x et z (profondeur) avec le temps pour faire une onde
        //    float wave = std::sin(pixel.x * 0.05f + pixel.z * 15.0f + time);
        //    float intensity = (wave * 0.5f) + 0.5f; // Ramène entre 0.0 et 1.0

        //    // Dégradé Cyan / Violet cyberpunk
        //    uint8_t r = static_cast<uint8_t>(intensity * 255.0f);
        //    uint8_t g = static_cast<uint8_t>((1.0f - intensity) * 150.0f);
        //    uint8_t b = 255;

        //    return (0xFF << 24) | (r << 16) | (g << 8) | b;
        //    });
     
        // 4. CUBE 3D 1w

        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(1.0f, 0.0f, -3.5f));
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

        // 5. CUBE 3D 2
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(0.0f, 0.0f, -2.5f));
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