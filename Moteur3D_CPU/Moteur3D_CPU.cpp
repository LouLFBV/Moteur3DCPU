#define TINYOBJLOADER_IMPLEMENTATION

#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <TINY_OBJ_LOADER/tiny_obj_loader.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <cstdint>
#include <iostream>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_main.h>

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
    float z;
    float r, g, b;
};

struct T_OBJVertex
{
    Vector4 position;
    float nx, ny, nz;
    float tx, ty;
    float r, g, b;
};

class T_OBJ
{
public:
    std::vector<T_OBJVertex> vertices;
};

std::vector<float> zbuffer(RENDER_WIDTH* RENDER_HEIGHT, 1.0f);

float Lerp(float a, float b, float t)  //Cours
{
    return (1.0f - t) * a + t * b;
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

void fillRowFast(std::vector<uint32_t>& fb, int y, const BordLigne& l, const BordLigne& r) {
    int w = std::max(r.x - l.x, 1); // largeur, au moins 1
    float dz = (r.z - l.z) / w, dr = (r.r - l.r) / w; // pas par pixel
    float dg = (r.g - l.g) / w, db = (r.b - l.b) / w;
    float z = l.z, cr = l.r, cg = l.g, cb = l.b;
    for (int x = l.x; x < r.x; x++) {
        if (x > 0 && x < RENDER_WIDTH) Plot(fb, zbuffer, x, y, z, cr, cg, cb);
        z += dz; cr += dr; cg += dg; cb += db; // une addition par attribut
    }
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

// Fonction lambda rapide pour faire tourner un sommet autour de l'axe Z
Vertex RotateVertex(Vertex v, float a)
{
    float x = v.position.x;
    float y = v.position.y;
    v.position.x = x * std::cos(a) - y * std::sin(a);
    v.position.y = x * std::sin(a) + y * std::cos(a);
    return v;
};

void Get_TOBJ(const char* path, T_OBJ& obj)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;

    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, path);

    if (!success)
        return;

    obj.vertices.clear();

    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                T_OBJVertex v_out{};

                //Positions
                v_out.position.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                v_out.position.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                v_out.position.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                v_out.position.w = 1.0f;

                //Normales
                if (idx.normal_index >= 0) {
                    v_out.nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    v_out.ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    v_out.nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                }

                //UVs (Textures)
                if (idx.texcoord_index >= 0) {
                    v_out.tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    v_out.ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                }

                //Couleur par défaut
                v_out.r = 0.8f; v_out.g = 0.8f; v_out.b = 0.8f;

                //Ajout au vecteur de la classe
                obj.vertices.push_back(v_out);
            }
            index_offset += fv;
        }
    }
}

int main(int argc, char* argv[])
{
    T_OBJ objModel;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Erreur SDL_Init : " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Moteur 3D CPU - Rendu OBJ", 800, 500, 0);
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

    const char* objPath = "External/Assets/base.obj";

    Get_TOBJ(objPath, objModel);
    bool loaded = !objModel.vertices.empty();

    if (loaded) {
        std::cout << "Obj Charge depuis : " << objPath << std::endl;
    }
    else {
        std::cerr << "Erreur : Impossible de charger " << objPath << std::endl;
    }

    bool running = true;
    SDL_Event event;

    float scale = 1.0f;
    float zOffset = 0.0f;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) running = false;
        }

        std::fill(framebuffer.begin(), framebuffer.end(), 0xFF000000);
        std::fill(zbuffer.begin(), zbuffer.end(), 1.0f);

        float angle = SDL_GetTicks() * 0.001f;

        if (loaded) {
            for (size_t i = 0; i + 2 < objModel.vertices.size(); i += 3) {

                // Extraction des 3 sommets de la face
                Vertex v0 = { objModel.vertices[i + 0].position, objModel.vertices[i + 0].r, objModel.vertices[i + 0].g, objModel.vertices[i + 0].b };
                Vertex v1 = { objModel.vertices[i + 1].position, objModel.vertices[i + 1].r, objModel.vertices[i + 1].g, objModel.vertices[i + 1].b };
                Vertex v2 = { objModel.vertices[i + 2].position, objModel.vertices[i + 2].r, objModel.vertices[i + 2].g, objModel.vertices[i + 2].b };

                // Application de l'échelle basique
                v0.position.x *= scale; v0.position.y *= scale; v0.position.z *= scale;
                v1.position.x *= scale; v1.position.y *= scale; v1.position.z *= scale;
                v2.position.x *= scale; v2.position.y *= scale; v2.position.z *= scale;

                // Rotation 3D basique autour de Y
                v0 = RotateVertex(v0, angle);
                v1 = RotateVertex(v1, angle);
                v2 = RotateVertex(v2, angle);

                // Ajustement Z neutre
                v0.position.z += zOffset;
                v1.position.z += zOffset;
                v2.position.z += zOffset;

                // Projection écran avec le ToScreen du cours
                ScreenVertex sv0 = ToScreen(v0);
                ScreenVertex sv1 = ToScreen(v1);
                ScreenVertex sv2 = ToScreen(v2);

                // Dessin du triangle
                DrawTriangle(framebuffer, sv0, sv1, sv2);
            }
        }

        SDL_UpdateTexture(
            framebufferTexture,
            nullptr,
            framebuffer.data(),
            RENDER_WIDTH * sizeof(uint32_t)
        );

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