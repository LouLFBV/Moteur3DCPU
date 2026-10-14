#include <SDL3/SDL.h>
#include <vector>
#include <cstdint>
#include <iostream>

const int RENDER_WIDTH = 320;
const int RENDER_HEIGHT = 200;

int main(int argc, char* argv[]) {
    // 1. Initialisation de SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) { // Vrai si l'initialisation a échoué
        std::cerr << "Erreur SDL_Init : " << SDL_GetError() << std::endl;
        return 1;
    }

    // 2. Création de la fenêtre (on la met plus grande, ex: 800x600, pour voir le scaling)
    SDL_Window* window = SDL_CreateWindow(
        "Moteur 3D CPU SDL3 - Rendu Basse Resolution",
        800, 500, // Largeur, Hauteur directement !
        0         // Pas de flag particulier pour l'instant
    );
    if (!window) {
        std::cerr << "Erreur SDL_CreateWindow : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 3. Création du Renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "Erreur SDL_CreateRenderer : " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 4. Création de la texture cible (notre "écran virtuel" en 320x200)
    // Format ARGB8888 : 32 bits par pixel (8 bits pour Alpha, Rouge, Vert, Bleu)
    SDL_Texture* framebufferTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        RENDER_WIDTH,
        RENDER_HEIGHT
    );

    // 5. Allocation de notre Framebuffer CPU (notre tableau de pixels en RAM)
    // Taille = Largeur x Hauteur entiers de 32 bits
    std::vector<uint32_t> framebuffer(RENDER_WIDTH * RENDER_HEIGHT);

    bool running = true;
    SDL_Event event;

    // Boucle principale de la démo
    while (running) {
        // --- A. Gestion des événements (Clavier, Croix de la fenêtre) ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                running = false;
            }
        }


        // --- B. Simulation de rendu CPU (On "peint" dans notre vecteur RAM) ---
        // On remplit le buffer avec des couleurs basées sur les coordonnées (x, y)
        for (int y = 0; y < RENDER_HEIGHT; ++y) {
            for (int x = 0; x < RENDER_WIDTH; ++x) {
                // Calcul de l'indice 1D à partir du 2D (x + y * largeur)
                int index = x + y * RENDER_WIDTH;

                // On fabrique une couleur ARGB (Alpha = FF, Rouge, Vert, Bleu)
                uint8_t red = static_cast<uint8_t>(x);       // Varie horizontalement
                uint8_t green = static_cast<uint8_t>(y);     // Varie verticalement
                uint8_t blue = 128;                          // Fixe

                // Format final : 0xAARRGGBB
                uint32_t color = (0xFF << 24) | (red << 16) | (green << 8) | blue;

                framebuffer[index] = color;
            }
        }

        // --- C. Transfert du CPU vers le GPU via SDL ---
        // 1. On envoie notre tableau de pixels vers la texture SDL
        SDL_UpdateTexture(
            framebufferTexture,
            nullptr,
            framebuffer.data(),
            RENDER_WIDTH * sizeof(uint32_t) // Pitch (taille d'une ligne en octets)
        );

        // 2. On nettoie le renderer de la fenêtre
        SDL_RenderClear(renderer);

        // 3. On copie notre texture basse résolution dans le renderer 
        // SDL se charge d'étirer l'image proprement à la taille de la fenêtre !
        SDL_RenderTexture(renderer, framebufferTexture, nullptr, nullptr);

        // 4. On affiche le tout à l'écran
        SDL_RenderPresent(renderer);
    }

    // 6. Nettoyage propre des ressources SDL
    SDL_DestroyTexture(framebufferTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}