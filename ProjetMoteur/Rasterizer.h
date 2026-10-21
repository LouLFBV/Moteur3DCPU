#pragma once
#include <vector>
#include <cstdint>
#include "Types.h"

class Rasterizer
{
public:
    Rasterizer(int width, int height);

    // Fonctions principales
    void Clear();
    void DrawTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3);
    void DrawSquare(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4);

    // Pour envoyer à la SDL
    const void* GetPixels() const;
    int GetPitch() const;

private:
    // Fonctions internes de dessin (cachées)
    ScreenVertex ToScreen(const Vertex& vertex);
    void Plot(int x, int y, float z, float r, float g, float b);
    void SortByY(ScreenVertex& sv1, ScreenVertex& sv2, ScreenVertex& sv3);
    bool MiddleIsRight(const ScreenVertex& sv1, const ScreenVertex& sv2, const ScreenVertex& sv3);
    void WalkEdge(const ScreenVertex& sv1, const ScreenVertex& sv2, BordLigne* out);
    void FillRows(int y0, int y1, const BordLigne left[], const BordLigne right[]);
    void FillRowFast(int y, const BordLigne& l, const BordLigne& r);

    // Les données de rendu
    int m_width;
    int m_height;
    std::vector<uint32_t> m_framebuffer;
    std::vector<float> m_zbuffer;
};