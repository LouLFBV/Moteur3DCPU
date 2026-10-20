#include <SDL3/SDL.h>
#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const int RENDER_WIDTH = 320;
const int RENDER_HEIGHT = 200;

struct Vertex //Cours
{
    glm::vec4 position;
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
    float z;
    float r, g, b;
};


std::vector<float> zbuffer(RENDER_WIDTH* RENDER_HEIGHT, 1.0f);

float Lerp(float a, float b, float t)  //Cours
{
    return (1.0f - t) * a + t * b;
}

glm::vec4 MultiplicationMatrixVector(const glm::mat4& matrix, glm::vec4 vector)  //Cours
{
    float in[4] = { vector.x, vector.y, vector.z, vector.w };
    float out[4] = { 0, 0, 0, 0 };

    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            out[row] += matrix[row][column] * in[column];

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

void SortByY(ScreenVertex& sv1, ScreenVertex& sv2, ScreenVertex& sv3)  //Cours
{
    if (sv1.y > sv2.y) std::swap(sv1, sv2);
    if (sv2.y > sv3.y) std::swap(sv2, sv3);
    if (sv1.y > sv2.y) std::swap(sv1, sv2);
}

bool MiddleIsRight(const ScreenVertex& sv1, const ScreenVertex& sv2, const ScreenVertex& sv3)  //Cours
{
    long cross = long(sv3.x - sv1.x) * (sv2.y - sv1.y) - long(sv3.y - sv1.y) * (sv2.x - sv1.x);
    return cross < 0;
}

void WalkEdge(const ScreenVertex& sv1, const ScreenVertex& sv2, BordLigne* out)  //Cours
{
    int distanceX = std::abs(sv2.x - sv1.x);
    int distanceY = sv2.y - sv1.y;
    if (distanceY == 0) return; // Arête horizontale ignorée

    int sensX = (sv2.x > sv1.x) ? 1 : -1;
    int err = distanceY / 2; // à expliquer
    int x = sv1.x;

    for (int y = sv1.y; y < sv2.y; ++y) {
        float t = float(y - sv1.y) / distanceY;
        if (y >= 0 && y < RENDER_HEIGHT)
            out[y] = { x, Lerp(sv1.z, sv2.z, t), Lerp(sv1.r, sv2.r, t), Lerp(sv1.g, sv2.g, t), Lerp(sv1.b, sv2.b, t) };

        for (err += distanceX; err > distanceY; err -= distanceY)
            x += sensX;
    }
}


void FillRows(std::vector<uint32_t>& fb, int y0, int y1, const BordLigne left[], const BordLigne right[])  //Cours à en discuter
{
    for (int y = std::max(y0, 0); y < std::min(y1, RENDER_HEIGHT); ++y) {
        const BordLigne& currentLeft = left[y];
        const BordLigne& currentRight = right[y];
        int width = currentRight.x - currentLeft.x;

        for (int x = std::max(currentLeft.x, 0); x < std::min(currentRight.x, RENDER_WIDTH); ++x) {
            float t = (width == 0) ? 0.0f : float(x - currentLeft.x) / width;
            Plot(fb, zbuffer, x, y, Lerp(currentLeft.z, currentRight.z, t), Lerp(currentLeft.r, currentRight.r, t), Lerp(currentLeft.g, currentRight.g, t), Lerp(currentLeft.b, currentRight.b, t));
        }
    }
}

void FillRowFast(std::vector<uint32_t>& fb, int y, const BordLigne& l, const BordLigne& r) {
    int w = std::max(r.x - l.x, 1); // largeur, au moins 1
    float dz = (r.z - l.z) / w, dr = (r.r - l.r) / w; // pas par pixel
    float dg = (r.g - l.g) / w, db = (r.b - l.b) / w;
    float z = l.z, cr = l.r, cg = l.g, cb = l.b;
    for (int x = l.x; x < r.x; x++) {
        if (x > 0 && x < RENDER_WIDTH) Plot(fb, zbuffer, x, y, z, cr, cg, cb);
        z += dz; cr += dr; cg += dg; cb += db; // une addition par attribut
    }
}

// invW[i] = 1 / w du sommet i; l = poids écran, corrigés sur place
void PerspectiveCorrect(const float invW[3], float l[3]) {
    float s = l[0] * invW[0] + l[1] * invW[1] + l[2] * invW[2];
    for (int i = 0; i < 3; i ++ ) l[i] = l[i] * invW[i] / s;
}

float PerspLerp(float a, float b, float wa, float wb, float t) {
    float num = (1 - t) * a / wa + t * b / wb; // v / w interpolé
        float den = (1 - t) / wa + t / wb; // 1 / w interpolé
        return num / den;
}


void DrawTriangle(std::vector<uint32_t>& fb, ScreenVertex sv1, ScreenVertex sv2, ScreenVertex sv3)  //Cours
{
    SortByY(sv1, sv2, sv3);
    static BordLigne longSide[RENDER_HEIGHT];
    static BordLigne shortSide[RENDER_HEIGHT];

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
void DrawSquare(std::vector<uint32_t>& fb, ScreenVertex sv1, ScreenVertex sv2, ScreenVertex sv3, ScreenVertex sv4)  
{
    DrawTriangle(fb, sv1, sv2, sv3);
    DrawTriangle(fb, sv1, sv3, sv4);
}

// Fonction lambda rapide pour faire tourner un sommet autour de l'axe Z
Vertex RotateVertex(Vertex v, float a)
{
    float x = v.position.x;
    float y = v.position.y;
    v.position.x = x * std::cos(a) - y * std::sin(a);
    v.position.y = x * std::sin(a) + y * std::cos(a);
    return v;
};

int main(int argc, char* argv[])
{
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

    std::vector<uint32_t> framebuffer(RENDER_WIDTH * RENDER_HEIGHT);

    bool running = true;
    SDL_Event event;

    //// Points Triangle
    //Vertex v0 = { {  0.0f,  0.5f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f };   // Haut (Rouge max)
    //Vertex v1 = { { -0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f };   // Bas-gauche (Vert max)
    //Vertex v2 = { {  0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f };   // Bas-droite (Bleu max)

    // Points Carré
    Vertex v0 = { {  -0.5f,  0.5f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f };   // Haut (Rouge max)
    Vertex v1 = { { -0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f };   // Bas-droite (Bleu max)
    Vertex v2 = { {  0.5f, -0.5f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f };   // Haut (Rouge max)
    Vertex v3 = { {  0.5f, 0.5f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f };   // Bas-droite (Bleu max)


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
        std::fill(framebuffer.begin(), framebuffer.end(), 0xFF000000);
        std::fill(zbuffer.begin(), zbuffer.end(), 1.0f);


        //// TRIANGLE FIXE
        //// Transformer les sommets
        //ScreenVertex sv0 = ToScreen(v0);
        //ScreenVertex sv1 = ToScreen(v1);
        //ScreenVertex sv2 = ToScreen(v2);

        //// Rasteriser le triangle
        //DrawTriangle(framebuffer, sv0, sv1, sv2);
        //// FIN TRIANGLE FIXE



        //// ROTATION DE TRIANGLE 
        //float angle = SDL_GetTicks() * 0.001f;

        //// Définir des sommets de base (par exemple centrés autour de (0,0))
        //// et appliquer une rotation 2D/3D en direct
        //Vertex baseV0 = { {  0.0f,  0.5f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f };
        //Vertex baseV1 = { { -0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f };
        //Vertex baseV2 = { {  0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f };

        //// On fait tourner nos sommets à chaque frame
        //Vertex v0 = RotateVertex(baseV0, angle);
        //Vertex v1 = RotateVertex(baseV1, angle);
        //Vertex v2 = RotateVertex(baseV2, angle);

        //// Transformer les sommets animés
        //ScreenVertex sv0 = ToScreen(v0);
        //ScreenVertex sv1 = ToScreen(v1);
        //ScreenVertex sv2 = ToScreen(v2);

        //// Rasteriser le triangle
        //DrawTriangle(framebuffer, sv0, sv1, sv2);
        //// FIN ROTATION

        // ROTATION DE Carré 
        float angle = SDL_GetTicks() * 0.001f;

        // Définir des sommets de base (par exemple centrés autour de (0,0))
        // et appliquer une rotation 2D/3D en direct
        Vertex baseV0 = { { -0.5f,  0.5f, 0.0f, 1.0f }, 1.0f, 0.0f, 0.0f }; // Haut-Gauche (Rouge)
        Vertex baseV1 = { { -0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f }; // Bas-Gauche (Vert)
        Vertex baseV2 = { {  0.5f, -0.5f, 0.0f, 1.0f }, 0.0f, 0.0f, 1.0f }; // Bas-Droite (Bleu)
        Vertex baseV3 = { {  0.5f,  0.5f, 0.0f, 1.0f }, 1.5f, 1.0f, 0.0f }; // Haut-Droite (Jaune/Autre)

        // On fait tourner nos sommets à chaque frame
        Vertex v0 = RotateVertex(baseV0, angle);
        Vertex v1 = RotateVertex(baseV1, angle);
        Vertex v2 = RotateVertex(baseV2, angle);
        Vertex v3 = RotateVertex(baseV3, angle);

        // Transformer les sommets animés
        ScreenVertex sv0 = ToScreen(v0);
        ScreenVertex sv1 = ToScreen(v1);
        ScreenVertex sv2 = ToScreen(v2);
        ScreenVertex sv3 = ToScreen(v3);
        
        // Rasteriser le triangle
        DrawSquare(framebuffer, sv0, sv1, sv2, sv3);
        // FIN ROTATION


        // Envoyer vers la texture SDL
        SDL_UpdateTexture(
            framebufferTexture,
            nullptr,
            framebuffer.data(),
            RENDER_WIDTH * sizeof(uint32_t)
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