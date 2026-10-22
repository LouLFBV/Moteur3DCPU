#include <SDL3/SDL.h>
#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "MathUtils.h"
#include "Rasterizer.h"

const int RENDER_WIDTH = 320;
const int RENDER_HEIGHT = 200;


int main(int argc, char* argv[])
{
	// Initialisation de SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Erreur SDL_Init : " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Moteur 3D CPU - Titre",
        800, 500,
        0
    );
    if (!window) {
        std::cerr << "Erreur SDL_CreateWindow : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "Erreur SDL_CreateRenderer : " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* framebufferTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        RENDER_WIDTH,
        RENDER_HEIGHT
    );
	// FIN Initialisation de SDL


    Rasterizer rasterizer(RENDER_WIDTH, RENDER_HEIGHT);

    bool isPerspective = true;
    bool running = true;
    SDL_Event event;


    float aspect = (float)RENDER_WIDTH / (float)RENDER_HEIGHT;
    float zNear = 0.1f;
    float zFar = 100.0f;

    while (running) {
        Uint64 startPerf = SDL_GetPerformanceCounter();

        // 1. Réinitialiser les compteurs de la frame précédente
        rasterizer.ResetStats();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE) {
                isPerspective = !isPerspective;
            }
        }

        // Effacer le framebuffer et le z-buffer
        rasterizer.Clear();

        glm::mat4 projection;
        if (isPerspective) {
            // Vue 3D classique (Perspective) : FOV de 60 degrés, ratio de l'écran, near plane à 0.1f, far plane à 100.0f
            projection = glm::perspective(glm::radians(60.0f), aspect, zNear, zFar);
        }
        else {
            // Vue Orthogonale (isométrique, aplatie)
            // On utilise le ratio de l'écran pour ne pas déformer l'image
            float taille = 2.0f; 
            projection = glm::ortho(-taille * aspect, taille * aspect, -taille, taille, zNear, zFar);
        }


        // FOND + PLAN 

        // 2. FOND (Ciel couvrant tout l'écran, reculé à Z = 0.99f)
        Vertex baseBackground[4] = {
            { { -1.0f,  1.0f, 0.99f, 1.0f }, 0.2f, 0.4f, 0.8f }, // Haut-Gauche (Bleu ciel)
            { { -1.0f, -1.0f, 0.99f, 1.0f }, 0.6f, 0.8f, 1.0f }, // Bas-Gauche (Plus clair)
            { {  1.0f, -1.0f, 0.99f, 1.0f }, 0.6f, 0.8f, 1.0f }, // Bas-Droite
            { {  1.0f,  1.0f, 0.99f, 1.0f }, 0.2f, 0.4f, 0.8f }  // Haut-Droite
        };
        rasterizer.DrawSquare(baseBackground[0], baseBackground[1], baseBackground[2], baseBackground[3]);

        // 3. PLAN / SOL (Couvrant la moitié basse de l'écran)
        Vertex basePlan[4] = {
            { { -1.0f,  0.0f, 0.95f, 1.0f }, 0.4f, 0.4f, 0.4f }, // Ligne d'horizon
            { { -1.0f, -1.0f, 0.95f, 1.0f }, 0.2f, 0.2f, 0.2f }, // Bas de l'écran
            { {  1.0f, -1.0f, 0.95f, 1.0f }, 0.2f, 0.2f, 0.2f },
            { {  1.0f,  0.0f, 0.95f, 1.0f }, 0.4f, 0.4f, 0.4f }
        };
        rasterizer.DrawSquare(basePlan[0], basePlan[1], basePlan[2], basePlan[3]);

        rasterizer.ClearZBuffer();

        //FIN FOND + PLAN





        //// TRIANGLE FIXE
        // 
        //// Points Triangle
        //Vertex v0 = { {  0.0f,  0.5f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f };   // Haut (Rouge max)
        //Vertex v1 = { { -0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f };   // Bas-gauche (Vert max)
        //Vertex v2 = { {  0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f };   // Bas-droite (Bleu max)

        //// Rasteriser le triangle
        //rasterizer.DrawTriangle(v0, v1, v2);
        //// FIN TRIANGLE FIXE



        float time = SDL_GetTicks() * 0.001f;


        //// ROTATION DE TRIANGLE 

        //// Définir des sommets de base (par exemple centrés autour de (0,0))
        //// et appliquer une rotation 2D/3D en direct
        //Vertex baseV0 = { {  0.0f,  0.5f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f };
        //Vertex baseV1 = { { -0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f };
        //Vertex baseV2 = { {  0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f };

        //// On fait tourner nos sommets à chaque frame
        //baseV0 = RotateVertex(baseV0, time);
        //baseV1 = RotateVertex(baseV1, time);
        //baseV2 = RotateVertex(baseV2, time);

        //// Rasteriser le triangle
        //rasterizer.DrawTriangle(baseV0, baseV1, baseV2);
        //// FIN ROTATION







        // ROTATION DE Carré 

        //// Définir des sommets de base (par exemple centrés autour de (0,0))
        //// et appliquer une rotation 2D/3D en direct
        //Vertex baseV0 = { { -0.5f,  0.5f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f }; // Haut-Gauche (Rouge)
        //Vertex baseV1 = { { -0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f }; // Bas-Gauche (Vert)
        //Vertex baseV2 = { {  0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f }; // Bas-Droite (Bleu)
        //Vertex baseV3 = { {  0.5f,  0.5f, 0.0f, 1.0f }, 1.5f, 1.0f, 0.0f }; // Haut-Droite (Jaune/Autre)

        //// On fait tourner nos sommets à chaque frame
        //baseV0 = RotateVertex(baseV0, -time); 
        //baseV1 = RotateVertex(baseV1, -time);
        //baseV2 = RotateVertex(baseV2, -time); 
        //baseV3 = RotateVertex(baseV3, -time);
        //
        //// Rasteriser le triangle
        //rasterizer.DrawSquare(baseV0, baseV1, baseV2, baseV3);
        // FIN ROTATION









        //// ROTATION CARRE ET TRIANGLE 
        //// // TRIANGLE (décalé à gauche)
        //{
        //    Vertex baseTri0 = { {  0.0f,  0.4f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f };
        //    Vertex baseTri1 = { { -0.4f, -0.4f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f };
        //    Vertex baseTri2 = { {  0.4f, -0.4f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f };

        //    // Rotation
        //    Vertex v0 = RotateVertex(baseTri0, time);
        //    Vertex v1 = RotateVertex(baseTri1, time);
        //    Vertex v2 = RotateVertex(baseTri2, time);

        //    // Translation (décalage vers la gauche sur l'axe X)
        //    float offsetX = -0.5f + std::sin(time) * 0.2f; // Petit mouvement de va-et-vient en prime
        //    v0.position.x += offsetX;
        //    v1.position.x += offsetX;
        //    v2.position.x += offsetX;
        //    rasterizer.DrawTriangle(v0, v1, v2);
        //}

        //// CARRÉ (décalé à droite)
        //{
        //    float angleSq = -time * 1.5f; // Rotation dans l'autre sens un peu plus vite
        //    Vertex baseSq0 = { { -0.35f,  0.35f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f };
        //    Vertex baseSq1 = { { -0.35f, -0.35f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f };
        //    Vertex baseSq2 = { {  0.35f, -0.35f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f };
        //    Vertex baseSq3 = { {  0.35f,  0.35f, 0.0f, 1.0f }, 0.0f, 1.0f, 1.0f };

        //    // Rotation
        //    Vertex v0 = RotateVertex(baseSq0, angleSq);
        //    Vertex v1 = RotateVertex(baseSq1, angleSq);
        //    Vertex v2 = RotateVertex(baseSq2, angleSq);
        //    Vertex v3 = RotateVertex(baseSq3, angleSq);

        //    // Translation (décalage vers la droite sur l'axe X)
        //    float offsetX = 0.5f;
        //    v0.position.x += offsetX;
        //    v1.position.x += offsetX;
        //    v2.position.x += offsetX;
        //    v3.position.x += offsetX;

        //    rasterizer.DrawSquare(v0, v1, v2, v3);
        //}
        //// FIN ROTATION CARRE ET TRIANGLE
        








        //// ROTATION + Déplacement sur l'axe Z
        //// --- 1. LE TRIANGLE ---
        //Vertex tri0 = { {  0.0f,  0.4f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f };
        //Vertex tri1 = { { -0.4f, -0.4f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f };
        //Vertex tri2 = { {  0.4f, -0.4f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f };

        //// A. Rotation
        //tri0 = RotateVertex(tri0, time);
        //tri1 = RotateVertex(tri1, time);
        //tri2 = RotateVertex(tri2, time);

        //// B. Translation (On décale à gauche en X, ET on recule en Z pour voir l'objet)
        //tri0.position.x -= 0.6f; tri0.position.z -= 2.5f;
        //tri1.position.x -= 0.6f; tri1.position.z -= 2.5f;
        //tri2.position.x -= 0.6f; tri2.position.z -= 2.5f;

        //// C. Projection perspective (Le w de vec4 va capturer la profondeur)
        //tri0.position = projection * tri0.position;
        //tri1.position = projection * tri1.position;
        //tri2.position = projection * tri2.position;

        //rasterizer.DrawTriangle(tri0, tri1, tri2);

        //// --- 2. LE CARRÉ ---
        //Vertex sq0 = { { -0.35f,  0.35f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f };
        //Vertex sq1 = { { -0.35f, -0.35f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f };
        //Vertex sq2 = { {  0.35f, -0.35f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f };
        //Vertex sq3 = { {  0.35f,  0.35f, 0.0f, 1.0f }, 0.0f, 1.0f, 1.0f };

        //// A. Rotation
        //sq0 = RotateVertex(sq0, -time); sq1 = RotateVertex(sq1, -time);
        //sq2 = RotateVertex(sq2, -time); sq3 = RotateVertex(sq3, -time);

        //// B. Translation (On décale à droite en X, ET on recule encore plus loin en Z !)
        //// Plus on recule (Z plus petit), plus l'objet paraîtra petit grâce à la perspective.
        //float reculCarre = -3.0f + std::sin(time) * 1.5f; // Faisons-le avancer et reculer !

        //sq0.position.z = reculCarre;
        //sq1.position.z = reculCarre;
        //sq2.position.z = reculCarre;
        //sq3.position.z = reculCarre;

        //// C. Projection perspective
        //sq0.position = projection * sq0.position;
        //sq1.position = projection * sq1.position;
        //sq2.position = projection * sq2.position;
        //sq3.position = projection * sq3.position;

        //rasterizer.DrawSquare(sq0, sq1, sq2, sq3);
        //// FIN ROTATION + Déplacement sur l'axe Z





        //DESSINER UN CUBE EN 3D 

        // Les 8 sommets d'un cube centré sur 0
        Vertex baseCube[8] = {
            // Face avant (Z = 0.5)
            { { -0.5f,  0.5f,  0.5f, 1.0f }, 1.0f, 0.0f, 0.0f }, // 0: Haut-Gauche (Rouge)
            { { -0.5f, -0.5f,  0.5f, 1.0f }, 0.0f, 1.0f, 0.0f }, // 1: Bas-Gauche (Vert)
            { {  0.5f, -0.5f,  0.5f, 1.0f }, 0.0f, 0.0f, 1.0f }, // 2: Bas-Droite (Bleu)
            { {  0.5f,  0.5f,  0.5f, 1.0f }, 1.0f, 1.0f, 0.0f }, // 3: Haut-Droite (Jaune)
            // Face arrière (Z = -0.5)
            { { -0.5f,  0.5f, -0.5f, 1.0f }, 0.0f, 1.0f, 1.0f }, // 4: Haut-Gauche arr (Cyan)
            { { -0.5f, -0.5f, -0.5f, 1.0f }, 1.0f, 0.0f, 1.0f }, // 5: Bas-Gauche arr (Magenta)
            { {  0.5f, -0.5f, -0.5f, 1.0f }, 1.0f, 1.0f, 1.0f }, // 6: Bas-Droite arr (Blanc)
            { {  0.5f,  0.5f, -0.5f, 1.0f }, 0.5f, 0.5f, 0.5f }  // 7: Haut-Droite arr (Gris)
        };

        // --- LE CUBE 3D 1 ---
        // 1. On crée une matrice d'identité
        glm::mat4 model = glm::mat4(1.0f);

        // 2. On recule le cube pour le voir
        model = glm::translate(model, glm::vec3(1.0f, 0.0f, -4.5f));

        // 3. On le fait tourner sur les axes X et Y en même temps 
        model = glm::rotate(model, time, glm::vec3(1.0f, 0.7f, 0.0f));

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
        model2 = glm::rotate(model2, time * 2.f, glm::vec3(-1.0f, 0.7f, 0.0f));

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
            { { 0.0f,  0.4f, -.0f, 1.0f}, 1.0f, 0.0f, 0.0f},
            { { -0.4f, -0.4f, -0.0f, 1.0f }, 0.0f, 1.0f, 0.0f },
            { {  0.4f, -0.4f, -.0f, 1.0f }, 0.0f, 0.0f, 1.0f } 
        };
         
        // 1. On crée une matrice d'identité
        glm::mat4 model3 = glm::mat4(1.0f);

        // 2. On recule le cube pour le voir
        model3 = glm::translate(model3, glm::vec3(-1.0f, 0.0f, -1.0f));

        // 3. On le fait tourner sur les axes X et Y en même temps 
        model3 = glm::rotate(model3, time * 1.5f, glm::vec3(-1.0f, -0.f, 0.0f));

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

        //FIN DESSINER UN CUBRE EN 3D


        
        // Envoyer vers la texture SDL
        SDL_UpdateTexture(
            framebufferTexture,
            nullptr,
            rasterizer.GetPixels(),
            rasterizer.GetPitch()
        );

        // Afficher à l'écran
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, framebufferTexture, nullptr, nullptr);
        SDL_RenderPresent(renderer);



        // GERER LES LOGS DE PERFORMANCES
        Uint64 endPerf = SDL_GetPerformanceCounter();
        float frameTimeMs = (endPerf - startPerf) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
        const RenderStats& stats = rasterizer.GetStats();

        static Uint32 lastLogTime = 0; 
        Uint32 currentTime = SDL_GetTicks();

        if (currentTime - lastLogTime >= 2000) {
            printf("CPU: %.2f ms |Triangles traites : %d | Triangles rasterises: %d | Pixels: %d | Threads: %d\n",
                frameTimeMs,
                stats.trianglesProcessed,
                stats.trianglesRasterized,
                stats.pixelsProcessed,
                stats.threadsUsed);

            lastLogTime = currentTime;
        }
    }

    SDL_DestroyTexture(framebufferTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}