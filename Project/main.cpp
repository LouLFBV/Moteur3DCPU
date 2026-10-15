#include <SDL3/SDL.h>
#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <cmath>

const int RENDER_WIDTH = 320;
const int RENDER_HEIGHT = 200;

// --- Structures mathématiques et de rendu ---
struct Vector4 //Cours
{
    float x, y, z, w;
};

struct Matrix4 //Cours
{
    float m[4][4];
};

struct Vertex //Cours
{
    Vector4 position;
    float r, g, b;
};

struct ScreenVertex  //Cours
{
    int x, y; // pixel
    float z; // profondeur NDC
    float r, g, b; // couleur
};


struct BordLigne //Cours
{
    int x;
    float z, r, g, b;
};

float Lerp(float a, float b, float t)  //Cours
{
    return a + (b - a) * t;
}

Vector4 MultiplicationMatrixVector(const Matrix4& matrix, Vector4 vector)  //Cours
{
    float in[4] = { vector.x, vector.y, vector.z, vector.w };
    float out[4] = { 0, 0, 0, 0 };

    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            out[row] += matrix.m[row][column] * in[column];

    return { out[0], out[1], out[2], out[3] };
}

// Projette un sommet 3D (espace de projection) vers l'écran 2D (pixels)
ScreenVertex ToScreen(const Vertex& vertex)
{
    // On calcule l'inverse de w pour remplacer les divisions par des multiplications (optimisation CPU)
    float inverseW = 1.0f / vertex.position.w;

    // Division perspective : on passe des coordonnées clip aux coordonnées NDC [-1, 1]
    float xndc = vertex.position.x * inverseW;
    float yndc = vertex.position.y * inverseW;
    float zndc = vertex.position.z * inverseW;

    return {
        // Transformation Viewport : on convertit les NDC en pixels de l'écran
        int(std::lround((xndc + 1) / 2 * RENDER_WIDTH)), // lround pour arrondir mathématiquement
        int(std::lround((1 - yndc) / 2 * RENDER_HEIGHT)), // Le (1 - y) inverse l'axe vertical pour l'écran
        zndc,
        vertex.r, vertex.g, vertex.b
    };
}

// --- Le Rasteriseur du cours (100% CPU) ---

void Plot(std::vector<uint32_t>& fb, int x, int y, float z, float r, float g, float b)  //Cours
{
    if (x >= 0 && x < RENDER_WIDTH && y >= 0 && y < RENDER_HEIGHT)
    {
        uint8_t ir = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
        uint8_t ig = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
        uint8_t ib = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));

        // Format ARGB (0xAARRGGBB)
        uint32_t color = (0xFF << 24) | (ir << 16) | (ig << 8) | ib;
        fb[x + y * RENDER_WIDTH] = color;
    }
}

void SortByY(ScreenVertex& a, ScreenVertex& b, ScreenVertex& c)  //Cours
{
    if (a.y > b.y) std::swap(a, b);
    if (b.y > c.y) std::swap(b, c);
    if (a.y > b.y) std::swap(a, b);
}

bool MiddleIsRight(const ScreenVertex& a, const ScreenVertex& b, const ScreenVertex& c)  //Cours
{
    long cross = long(c.x - a.x) * (b.y - a.y) - long(c.y - a.y) * (b.x - a.x);
    return cross < 0;
}

void WalkEdge(const ScreenVertex& a, const ScreenVertex& b, BordLigne* out)  //Cours
{
    int dx = std::abs(b.x - a.x), dy = b.y - a.y;
    if (dy == 0) return; // Arête horizontale ignorée
    int sx = (b.x > a.x) ? 1 : -1, err = dy / 2, x = a.x;
    for (int y = a.y; y < b.y; ++y) {
        float t = float(y - a.y) / dy;
        if (y >= 0 && y < RENDER_HEIGHT)
            out[y] = { x, Lerp(a.z, b.z, t), Lerp(a.r, b.r, t),
                       Lerp(a.g, b.g, t), Lerp(a.b, b.b, t) };
        for (err += dx; err > dy; err -= dy) x += sx;
    }
}

void FillRows(std::vector<uint32_t>& fb, int y0, int y1, const BordLigne* left, const BordLigne* right)  //Cours
{
    for (int y = std::max(y0, 0); y < std::min(y1, RENDER_HEIGHT); ++y) {
        const BordLigne& l = left[y], & r = right[y];
        int w = r.x - l.x;
        for (int x = std::max(l.x, 0); x < std::min(r.x, RENDER_WIDTH); ++x) {
            float t = (w == 0) ? 0.0f : float(x - l.x) / w;
            Plot(fb, x, y, Lerp(l.z, r.z, t), Lerp(l.r, r.r, t),
                Lerp(l.g, r.g, t), Lerp(l.b, r.b, t));
        }
    }
}

void DrawTriangle(std::vector<uint32_t>& fb, ScreenVertex a, ScreenVertex b, ScreenVertex c)  //Cours
{
    SortByY(a, b, c);
    static BordLigne longSide[RENDER_HEIGHT], shortSide[RENDER_HEIGHT];

    // Nettoyage rapide des tableaux statiques
    std::fill_n(longSide, RENDER_HEIGHT, BordLigne{ 0, 0, 0, 0, 0 });
    std::fill_n(shortSide, RENDER_HEIGHT, BordLigne{ 0, 0, 0, 0, 0 });

    WalkEdge(a, c, longSide);
    WalkEdge(a, b, shortSide);
    WalkEdge(b, c, shortSide);

    bool right = MiddleIsRight(a, b, c);
    FillRows(fb, a.y, c.y, right ? longSide : shortSide,
        right ? shortSide : longSide);
}

int main(int argc, char* argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Erreur SDL_Init : " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Moteur 3D CPU - Rasteriseur Personnel",
        800, 500,
        0
    );
    if (!window) {
        std::cerr << "Erreur SDL_CreateWindow : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Création du "Renderer" (le gestionnaire de dessin)
    // Il fait le lien entre notre code et la fenêtre pour afficher des pixels/textures.
    // Le 'nullptr' laisse SDL choisir automatiquement le meilleur pilote matériel (DirectX/OpenGL/Vulkan)
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

    std::vector<uint32_t> framebuffer(RENDER_WIDTH * RENDER_HEIGHT);

    bool running = true;
    SDL_Event event;

    // Définition de 3 sommets 3D de test (dans l'espace clip pour l'instant)
    Vertex v0 = { 
        {  0.0f,  0.5f, 0.0f, 1.0f },//Position
        255.0f, 0.0f, 0.0f //Couleur
    };   // Sommet haut (Rouge)

    Vertex v1 = { 
        { -0.5f, -0.5f, 0.0f, 1.0f },//Position
        0.0f, 255.0f, 0.0f //Couleur
    };   // Sommet bas-gauche (Vert)

    Vertex v2 = { 
        {  0.5f, -0.5f, 0.0f, 1.0f },//Position
        0.0f, 0.0f, 255.0f //Couleur
    };   // Sommet bas-droite (Bleu)


    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                running = false;
            }
        }

        // 1. Effacer le framebuffer (fond noir : 0xFF000000)
        std::fill(framebuffer.begin(), framebuffer.end(), 0xFF000000);

        // 2. Transformer les sommets 3D en sommets écran 2D (SV)
        ScreenVertex sv0 = ToScreen(v0);
        ScreenVertex sv1 = ToScreen(v1);
        ScreenVertex sv2 = ToScreen(v2);

        // 3. Rasteriser notre triangle entièrement sur le CPU !
        DrawTriangle(framebuffer, sv0, sv1, sv2);

        // 4. Envoyer le framebuffer CPU vers la texture GPU SDL
        SDL_UpdateTexture(
            framebufferTexture,
            nullptr,
            framebuffer.data(),
            RENDER_WIDTH * sizeof(uint32_t)
        );

        // 5. Afficher à l'écran
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