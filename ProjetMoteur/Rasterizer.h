#pragma once

#include "Types.h"
#include <vector>
#include <functional>
#include <cstdint>

using PixelShaderCallback = std::function<uint32_t(const BordLigne&)>;

class Rasterizer {
public:
    Rasterizer(int width, int height);
    ~Rasterizer();

    void Clear();
    void ClearZBuffer();

    void RasterizeTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2,
        int tileMinY, int tileMaxY,
        const PixelShaderCallback& pixelShader,
        std::vector<BordLigne>& longSide,
        std::vector<BordLigne>& shortSide);

    const void* GetPixels() const;
    int GetPitch() const;
    int GetHeight() const;
    int GetWidth() const;

    void ResetStats();
    RenderStats& GetStats();
    const RenderStats& GetStats() const;

private:
    int m_width;
    int m_height;

    std::vector<uint32_t> m_framebuffer;
    std::vector<float> m_zbuffer;

    RenderStats m_stats;

    ScreenVertex ToScreen(const Vertex& vertex);
    void SortByY(ScreenVertex& sv1, ScreenVertex& sv2, ScreenVertex& sv3);
    bool MiddleIsRight(const ScreenVertex& sv1, const ScreenVertex& sv2, const ScreenVertex& sv3);
    void WalkEdge(const ScreenVertex& sv1, const ScreenVertex& sv2, BordLigne* out) const;

    void FillRows(int y0, int y1, const BordLigne* left, const BordLigne* right,
        int tileMinY, int tileMaxY, const PixelShaderCallback& pixelShader);
    void FillRowFast(int y, const BordLigne& left, const BordLigne& right,
        const PixelShaderCallback& pixelShader);

    void Plot(int x, int y, float z, uint32_t color);
};