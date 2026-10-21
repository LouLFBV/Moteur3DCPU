#include <SDL3/SDL.h>
#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

    bool running = true;
    SDL_Event event;



    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                running = false;
            }
        }

        // Effacer le framebuffer et le z-buffer
        rasterizer.Clear();


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





        // ROTATION CARRE ET TRIANGLE 
        // // --- 1. ANIMATION ET RENDU DU TRIANGLE (décalé à gauche) ---
        {
            Vertex baseTri0 = { {  0.0f,  0.4f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f };
            Vertex baseTri1 = { { -0.4f, -0.4f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f };
            Vertex baseTri2 = { {  0.4f, -0.4f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f };

            // Rotation
            Vertex v0 = RotateVertex(baseTri0, time);
            Vertex v1 = RotateVertex(baseTri1, time);
            Vertex v2 = RotateVertex(baseTri2, time);

            // Translation (décalage vers la gauche sur l'axe X)
            float offsetX = -0.5f + std::sin(time) * 0.2f; // Petit mouvement de va-et-vient en prime
            v0.position.x += offsetX;
            v1.position.x += offsetX;
            v2.position.x += offsetX;
            rasterizer.DrawTriangle(v0, v1, v2);
        }

        // --- 2. ANIMATION ET RENDU DU CARRÉ (décalé à droite) ---
        {
            float angleSq = -time * 1.5f; // Rotation dans l'autre sens un peu plus vite
            Vertex baseSq0 = { { -0.35f,  0.35f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f };
            Vertex baseSq1 = { { -0.35f, -0.35f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f };
            Vertex baseSq2 = { {  0.35f, -0.35f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f };
            Vertex baseSq3 = { {  0.35f,  0.35f, 0.0f, 1.0f }, 0.0f, 1.0f, 1.0f };

            // Rotation
            Vertex v0 = RotateVertex(baseSq0, angleSq);
            Vertex v1 = RotateVertex(baseSq1, angleSq);
            Vertex v2 = RotateVertex(baseSq2, angleSq);
            Vertex v3 = RotateVertex(baseSq3, angleSq);

            // Translation (décalage vers la droite sur l'axe X)
            float offsetX = 0.5f;
            v0.position.x += offsetX;
            v1.position.x += offsetX;
            v2.position.x += offsetX;
            v3.position.x += offsetX;

            rasterizer.DrawSquare(v0, v1, v2, v3);
        }
        // FIN ROTATION CARRE ET TRIANGLE
        
        
        
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
    }

    SDL_DestroyTexture(framebufferTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}