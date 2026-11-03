#pragma once

#include "Types.h"
#include "Texture.h"
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

    void RasterizeTriangleBox(const Vertex& a, const Vertex& b, const Vertex& c,
        int minY, int maxY,
        const PixelShaderCallback& pixelShaderCallback);

    void DrawSubImage2D(int screenX, int screenY, int width, int height,
        const Texture& texture, int srcX, int srcY, uint32_t textColor);

    void SetAlphaBlendingEnabled(bool enabled);
    void SetDepthWriteEnabled(bool enabled);

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

    bool m_alphaBlendingEnabled = false;
    bool m_depthWriteEnabled = true;

    bool WritePixelColor(int index, uint32_t color);

    ScreenVertex ToScreen(const Vertex& vertex);
    void SortByY(ScreenVertex& sv1, ScreenVertex& sv2, ScreenVertex& sv3);
    bool MiddleIsRight(const ScreenVertex& sv1, const ScreenVertex& sv2, const ScreenVertex& sv3);
    void WalkEdge(const ScreenVertex& sv1, const ScreenVertex& sv2, BordLigne* out) const;

    void FillRows(int y0, int y1, const BordLigne* left, const BordLigne* right, int tileMinY, int tileMaxY, const PixelShaderCallback& pixelShader);
    void FillRowFast(int y, const BordLigne& left, const BordLigne& right, const PixelShaderCallback& pixelShader);

    // Calcule la boîte englobante et la contraint aux dimensions de l'écran
    Box BoundingBox(const Vertex& a, const Vertex& b, const Vertex& c, int width, int height);

    // Fonction d'interpolation basée sur les poids barycentriques l
    float Mix3(float a, float b, float c, const float l[3]);

    // Calcule l'aire d'un triangle défini par 3 points 2D (utilisé pour les coordonnées barycentriques)
    float EdgeFunction(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);

    // Remplit le tableau 'l' avec les poids barycentriques. Retourne false si le triangle est dégénéré.
    bool Barycentric(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& p, float l[3]);


    void Plot(int x, int y, float z, uint32_t color);
};