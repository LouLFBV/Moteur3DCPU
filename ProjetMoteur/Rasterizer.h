#pragma once
#include <vector>
#include <cstdint>
#include "Types.h"

struct RenderStats {
    float frameTimeMs = 0.0f;
    int trianglesProcessed = 0;
    int trianglesRasterized = 0;
    int pixelsProcessed = 0;
    int threadsUsed = 1; 
};

class Rasterizer
{
public:
    Rasterizer(int width, int height);

    void ResetStats();
    const RenderStats& GetStats() const;

    void Clear();
    void ClearZBuffer();
    void DrawTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3);
    void DrawSquare(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const Vertex& vertex4);
    void DrawCube(const Cube& cube);

    const void* GetPixels() const;
    int GetPitch() const;

private:
    ScreenVertex ToScreen(const Vertex& vertex);
    void Plot(int x, int y, float z, float r, float g, float b);
    void SortByY(ScreenVertex& sv1, ScreenVertex& sv2, ScreenVertex& sv3);
    bool MiddleIsRight(const ScreenVertex& sv1, const ScreenVertex& sv2, const ScreenVertex& sv3);
    void WalkEdge(const ScreenVertex& sv1, const ScreenVertex& sv2, BordLigne* out);
    void FillRows(int y0, int y1, const BordLigne* left, const BordLigne* right);
    void FillRowFast(int y, const BordLigne& l, const BordLigne& r);
    Vertex Mix(const Vertex& vertex1, const Vertex& vertex2, float t);
    float NearDist(const Vertex& vertex);
    Vertex CrossNear(const Vertex& p, const Vertex& q);
    int ClipNear(const Vertex in[3], Vertex poly[4]);
    void RasterizeTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3);

    int m_width;
    int m_height;
    std::vector<uint32_t> m_framebuffer;
    std::vector<float> m_zbuffer;

    RenderStats m_stats;
};