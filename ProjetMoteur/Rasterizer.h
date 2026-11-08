#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include "Types.h"
#include "Texture.h"

using PixelShaderCallback = std::function<uint32_t(const BordLigne&)>;

class Rasterizer
{
public:
    Rasterizer(int width, int height);

    void Clear();
    void ClearZBuffer();

    void RasterizeTriangle(
        const Vertex& vertex0,
        const Vertex& vertex1,
        const Vertex& vertex2,
        int tileMinY,
        int tileMaxY,
        const PixelShaderCallback& pixelShader,
        std::vector<BordLigne>& longSide,
        std::vector<BordLigne>& shortSide
    );

    void RasterizeTriangleBox(
        const Vertex& vertex0,
        const Vertex& vertex1,
        const Vertex& vertex2,
        int minY,
        int maxY,
        const PixelShaderCallback& pixelShaderCallback
    );

    void DrawSubImage2D(
        int screenX,
        int screenY,
        int width,
        int height,
        const Texture& texture,
        int srcX,
        int srcY,
        uint32_t textColor
    );

    void SetAlphaBlendingEnabled(bool enabled);
    void SetDepthTestEnabled(bool enabled);
    void SetDepthWriteEnabled(bool enabled);

    uint32_t* GetPixels();
    const void* GetPixels() const;

    int GetPitch() const;
    int GetWidth() const;
    int GetHeight() const;

    void ResetStats();

    RenderStats& GetStats();
    const RenderStats& GetStats() const;

private:
    int m_width;
    int m_height;

    std::vector<uint32_t> m_framebuffer;
    std::vector<float> m_zbuffer;

    RenderStats m_stats;

    bool m_alphaBlendingEnabled = false;
    bool m_depthWriteEnabled = true;
    bool m_depthTestEnabled = true;

    bool WritePixelColor(int index, uint32_t color);

    void Plot(int x, int y, float z, uint32_t color);

    ScreenVertex ToScreen(const Vertex& vertex);

    void SortByY(
        ScreenVertex& screenVertex1,
        ScreenVertex& screenVertex2,
        ScreenVertex& screenVertex3
    );

    bool MiddleIsRight(
        const ScreenVertex& screenVertex1,
        const ScreenVertex& screenVertex2,
        const ScreenVertex& screenVertex3
    );

    void WalkEdge(
        const ScreenVertex& screenVertex1,
        const ScreenVertex& screenVertex2,
        BordLigne* out
    ) const;

    void FillRows(
        int y0,
        int y1,
        const BordLigne* left,
        const BordLigne* right,
        int tileMinY,
        int tileMaxY,
        const PixelShaderCallback& pixelShader
    );

    void FillRowFast(
        int y,
        const BordLigne& left,
        const BordLigne& right,
        const PixelShaderCallback& pixelShader
    );

    float Mix3(float value0, float value1, float value2, const float weights[3]);

    float EdgeFunction(
        const glm::vec2& pointA,
        const glm::vec2& pointB,
        const glm::vec2& pointC
    );
};