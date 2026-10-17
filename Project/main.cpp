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
    return (1.0f - t) * a + t*b;
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
        int(std::lround((xndc + 1) / 2 * RENDER_WIDTH)), 
        int(std::lround((1 - yndc) / 2 * RENDER_HEIGHT)), 
        zndc,
        vertex.r, vertex.g, vertex.b
    };
}

// --- Le Rasteriseur du cours (100% CPU) ---

// 1. Déclaration du Z-Buffer global
std::vector<float> zbuffer(RENDER_WIDTH * RENDER_HEIGHT, 1.0f);

// 2. La fonction Plot mise à jour avec le test de profondeur du cours
void Plot(std::vector<uint32_t>& fb, std::vector<float>& zb, int x, int y, float z, float r, float g, float b)
{
    if (x >= 0 && x < RENDER_WIDTH && y >= 0 && y < RENDER_HEIGHT)
    {
        int i = x + y * RENDER_WIDTH; // Index 1D du pixel

        // Test de profondeur (Z-Buffer) : 
        if (z >= zb[i]) return;

        // Sinon, on met à jour la profondeur dans le z-buffer
        zb[i] = z;

        // Conversion des couleurs 
        uint8_t ir = static_cast<uint8_t>(std::clamp(255.0f * r + 0.5f, 0.0f, 255.0f));
        uint8_t ig = static_cast<uint8_t>(std::clamp(255.0f * g + 0.5f, 0.0f, 255.0f));
        uint8_t ib = static_cast<uint8_t>(std::clamp(255.0f * b + 0.5f, 0.0f, 255.0f));

        // Écriture de la couleur dans le framebuffer ARGB
        fb[i] = (0xFF << 24) | (ir << 16) | (ig << 8) | ib;
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

void WalkEdge(const ScreenVertex& sv1, const ScreenVertex& sv2, BordLigne* out)  //Cours
{
    int distanceX = std::abs(sv2.x - sv1.x);
    int distanceY = sv2.y - sv1.y; 
    if (distanceY == 0) return; // Arête horizontale ignorée

    int sensX = (sv2.x > sv1.x) ? 1 : -1;
    int err = distanceY / 2;
    int x = sv1.x;

    for (int y = sv1.y; y < sv2.y; ++y) { 
        float t = float(y - sv1.y) / distanceY;
        if (y >= 0 && y < RENDER_HEIGHT)
            out[y] = { x, Lerp(sv1.z, sv2.z, t), Lerp(sv1.r, sv2.r, t),
                       Lerp(sv1.g, sv2.g, t), Lerp(sv1.b, sv2.b, t) };
        for (err += distanceX; err > distanceY; err -= distanceY)
            x += sensX;
    }
}

void FillRows(std::vector<uint32_t>& fb, int y0, int y1, const BordLigne* left, const BordLigne* right)  //Cours
{
    for (int y = std::max(y0, 0); y < std::min(y1, RENDER_HEIGHT); ++y) {
        const BordLigne& l = left[y], & r = right[y];
        int w = r.x - l.x;
        for (int x = std::max(l.x, 0); x < std::min(r.x, RENDER_WIDTH); ++x) {
            float t = (w == 0) ? 0.0f : float(x - l.x) / w;
            Plot(fb, zbuffer, x, y, Lerp(l.z, r.z, t), Lerp(l.r, r.r, t),
                Lerp(l.g, r.g, t), Lerp(l.b, r.b, t));
        }
    }
}

void DrawTriangle(std::vector<uint32_t>& fb, ScreenVertex sv1, ScreenVertex sv2, ScreenVertex sv3)  //Cours
{
    SortByY(sv1, sv2, sv3);
    static BordLigne longSide[RENDER_HEIGHT], shortSide[RENDER_HEIGHT];

    // Nettoyage rapide des tableaux statiques
    std::fill_n(longSide, RENDER_HEIGHT, BordLigne{ 0, 0, 0, 0, 0 });
    std::fill_n(shortSide, RENDER_HEIGHT, BordLigne{ 0, 0, 0, 0, 0 });

    WalkEdge(sv1, sv3, longSide);
    WalkEdge(sv1, sv2, shortSide);
    WalkEdge(sv2, sv3, shortSide);

    bool right = MiddleIsRight(sv1, sv2, sv3);
    FillRows(fb, sv1.y, sv3.y, right ? longSide : shortSide,
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

    Vertex v0 = { {  0.0f,  0.5f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f };   // Haut (Rouge max)
    Vertex v1 = { { -0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f };   // Bas-gauche (Vert max)
    Vertex v2 = { {  0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f };   // Bas-droite (Bleu max)

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                running = false;
            }
        }

        // 1. Effacer le framebuffer et le z-buffer
        std::fill(framebuffer.begin(), framebuffer.end(), 0xFF000000); 
        std::fill(zbuffer.begin(), zbuffer.end(), 1.0f);               

        // 2. Transformer les sommets
        ScreenVertex sv0 = ToScreen(v0);
        ScreenVertex sv1 = ToScreen(v1);
        ScreenVertex sv2 = ToScreen(v2);

        // 3. Rasteriser le triangle
        DrawTriangle(framebuffer, sv0, sv1, sv2);

        // 4. Envoyer vers la texture SDL
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