#include "Rasterizer.h"
#include "MathUtils.h"
#include <algorithm>
#include <cmath>

Rasterizer::Rasterizer(int width, int height)
    : m_width(width), m_height(height)
{
    m_framebuffer.resize(width * height, 0xFF000000);
    m_zbuffer.resize(width * height, 1.0f);
}

Rasterizer::~Rasterizer()
{}

void Rasterizer::ResetStats()
{
    m_stats.trianglesProcessed = 0;
    m_stats.trianglesRasterized = 0;
    m_stats.pixelsProcessed.store(0);
    m_stats.frameTimeMs = 0.0f;
    m_stats.threadsUsed = 0;
}

const RenderStats& Rasterizer::GetStats() const
{
    return m_stats;
}

RenderStats& Rasterizer::GetStats()
{
    return m_stats;
}

void Rasterizer::Clear()
{
    std::fill(m_framebuffer.begin(), m_framebuffer.end(), 0xFF000000);
    std::fill(m_zbuffer.begin(), m_zbuffer.end(), 1.0f);
}

void Rasterizer::ClearZBuffer()
{
    std::fill(m_zbuffer.begin(), m_zbuffer.end(), 1.0f);
}

const void* Rasterizer::GetPixels() const
{
    return m_framebuffer.data();
}

int Rasterizer::GetPitch() const
{
    return m_width * sizeof(uint32_t);
}

int Rasterizer::GetHeight() const
{
    return m_height;
}

int Rasterizer::GetWidth() const
{
    return m_width;
}

ScreenVertex Rasterizer::ToScreen(const Vertex& vertex)
{
    float inverseW = 1.0f / vertex.position.w;
    float xndc = vertex.position.x * inverseW;
    float yndc = vertex.position.y * inverseW;
    float zndc = vertex.position.z * inverseW;

    return {
        int(std::lround((xndc + 1) / 2 * m_width)),
        int(std::lround((1 - yndc) / 2 * m_height)),
        zndc, vertex.r, vertex.g, vertex.b,
        inverseW,
        vertex.uv.x, vertex.uv.y,
        vertex.normal.x, vertex.normal.y, vertex.normal.z,
        {
            vertex.worldPos.x,
            vertex.worldPos.y,
            vertex.worldPos.z
        }
    };
}

void Rasterizer::Plot(int x, int y, float z, uint32_t color)
{
    m_stats.pixelsProcessed++;

    if (x >= 0 && x < m_width && y >= 0 && y < m_height)
    {
        int i = x + y * m_width;
        if (z >= m_zbuffer[i]) return;
        m_zbuffer[i] = z;

        m_framebuffer[i] = color;
    }
}

void Rasterizer::SortByY(ScreenVertex& screenVertex1, ScreenVertex& screenVertex2, ScreenVertex& screenVertex3)
{
    if (screenVertex1.y > screenVertex2.y) std::swap(screenVertex1, screenVertex2);
    if (screenVertex2.y > screenVertex3.y) std::swap(screenVertex2, screenVertex3);
    if (screenVertex1.y > screenVertex2.y) std::swap(screenVertex1, screenVertex2);
}

bool Rasterizer::MiddleIsRight(const ScreenVertex& screenVertex1, const ScreenVertex& screenVertex2, const ScreenVertex& screenVertex3)
{
    long cross = long(screenVertex3.x - screenVertex1.x) * (screenVertex2.y - screenVertex1.y) - long(screenVertex3.y - screenVertex1.y) * (screenVertex2.x - screenVertex1.x);
    return cross < 0;
}

void Rasterizer::WalkEdge(const ScreenVertex& screenVertex1, const ScreenVertex& screenVertex2, BordLigne* out) const
{
    int distanceX = std::abs(screenVertex2.x - screenVertex1.x);
    int distanceY = screenVertex2.y - screenVertex1.y;
    if (distanceY == 0) return;

    int sensX = (screenVertex2.x > screenVertex1.x) ? 1 : -1;
    int err = distanceY / 2;
    int x = screenVertex1.x;

    for (int y = screenVertex1.y; y < screenVertex2.y; ++y) {
        float t = float(y - screenVertex1.y) / distanceY;
        if (y >= 0 && y < m_height) {
            out[y] = {
                x,
                Lerp(screenVertex1.z, screenVertex2.z, t),
                Lerp(screenVertex1.r, screenVertex2.r, t), Lerp(screenVertex1.g, screenVertex2.g, t), Lerp(screenVertex1.b, screenVertex2.b, t),
                Lerp(screenVertex1.invW, screenVertex2.invW, t),
                Lerp(screenVertex1.u, screenVertex2.u, t), Lerp(screenVertex1.v, screenVertex2.v, t),
                Lerp(screenVertex1.nx, screenVertex2.nx, t), Lerp(screenVertex1.ny, screenVertex2.ny, t), Lerp(screenVertex1.nz, screenVertex2.nz, t),
                {
                    Lerp(screenVertex1.worldPos.x, screenVertex2.worldPos.x, t),
                    Lerp(screenVertex1.worldPos.y, screenVertex2.worldPos.y, t),
                    Lerp(screenVertex1.worldPos.z, screenVertex2.worldPos.z, t)
                }
            };
        }
        for (err += distanceX; err > distanceY; err -= distanceY)
            x += sensX;
    }
}

void Rasterizer::FillRows(int y0, int y1, const BordLigne* left, const BordLigne* right, int tileMinY, int tileMaxY, const PixelShaderCallback& pixelShader)
{
    int startY = std::max({ y0, 0, tileMinY });
    int endY = std::min({ y1, m_height, tileMaxY });

    for (int y = startY; y < endY; ++y)
    {
        FillRowFast(y, left[y], right[y], pixelShader);
    }
}

void Rasterizer::FillRowFast(int y, const BordLigne& left, const BordLigne& right, const PixelShaderCallback& pixelShader)
{
    int width = std::max(right.x - left.x, 1);

    float invWidth = 1.0f / width;

    float deltaZ = (right.z - left.z) * invWidth;
    float deltaR = (right.r - left.r) * invWidth;
    float deltaG = (right.g - left.g) * invWidth;
    float deltaB = (right.b - left.b) * invWidth;

    float deltaU = (right.u - left.u) * invWidth;
    float deltaV = (right.v - left.v) * invWidth;
    float deltaNx = (right.nx - left.nx) * invWidth;
    float deltaNy = (right.ny - left.ny) * invWidth;
    float deltaNz = (right.nz - left.nz) * invWidth;

    float deltaWorldX = (right.worldPos.x - left.worldPos.x) * invWidth;
    float deltaWorldY = (right.worldPos.y - left.worldPos.y) * invWidth;
    float deltaWorldZ = (right.worldPos.z - left.worldPos.z) * invWidth;

    BordLigne current = left;

    for (int x = left.x; x < right.x; x++) {
        if (x >= 0 && x < m_width)
        {
            current.x = x;

            uint32_t finalColor = pixelShader ? pixelShader(current) : 0xFFFFFFFF;
            Plot(x, y, current.z, finalColor);
        }

        current.z += deltaZ;
        current.r += deltaR;
        current.g += deltaG;
        current.b += deltaB;

        current.u += deltaU;
        current.v += deltaV;
        current.nx += deltaNx;
        current.ny += deltaNy;
        current.nz += deltaNz;

        current.worldPos.x += deltaWorldX;
        current.worldPos.y += deltaWorldY;
        current.worldPos.z += deltaWorldZ;
    }
}

void Rasterizer::RasterizeTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3,
    int tileMinY, int tileMaxY,
    const PixelShaderCallback& pixelShader,
    std::vector<BordLigne>& longSide,
    std::vector<BordLigne>& shortSide)
{
    m_stats.trianglesRasterized++;

    ScreenVertex sv1 = ToScreen(vertex1);
    ScreenVertex sv2 = ToScreen(vertex2);
    ScreenVertex sv3 = ToScreen(vertex3);

    SortByY(sv1, sv2, sv3);

    WalkEdge(sv1, sv3, longSide.data());
    WalkEdge(sv1, sv2, shortSide.data());
    WalkEdge(sv2, sv3, shortSide.data());

    bool right = MiddleIsRight(sv1, sv2, sv3);
    FillRows(sv1.y, sv3.y, right ? longSide.data() : shortSide.data(), right ? shortSide.data() : longSide.data(), tileMinY, tileMaxY, pixelShader);
}