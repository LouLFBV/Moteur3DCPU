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

void Rasterizer::Clear()
{
    std::fill(m_framebuffer.begin(), m_framebuffer.end(), 0xFF000000);
    std::fill(m_zbuffer.begin(), m_zbuffer.end(), 1.0f);
}

const void* Rasterizer::GetPixels() const { return m_framebuffer.data(); }
int Rasterizer::GetPitch() const { return m_width * sizeof(uint32_t); }

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
        inverseW // <-- Prêt pour la perspective !
    };
}

void Rasterizer::Plot(int x, int y, float z, float r, float g, float b)
{
    if (x >= 0 && x < m_width && y >= 0 && y < m_height)
    {
        int i = x + y * m_width;
        if (z >= m_zbuffer[i]) return;
        m_zbuffer[i] = z;

        uint8_t ir = static_cast<uint8_t>(std::clamp(255.0f * r + 0.5f, 0.0f, 255.0f));
        uint8_t ig = static_cast<uint8_t>(std::clamp(255.0f * g + 0.5f, 0.0f, 255.0f));
        uint8_t ib = static_cast<uint8_t>(std::clamp(255.0f * b + 0.5f, 0.0f, 255.0f));

        m_framebuffer[i] = (0xFF << 24) | (ir << 16) | (ig << 8) | ib;
    }
}

void Rasterizer::SortByY(ScreenVertex& sv1, ScreenVertex& sv2, ScreenVertex& sv3)
{
    if (sv1.y > sv2.y) std::swap(sv1, sv2);
    if (sv2.y > sv3.y) std::swap(sv2, sv3);
    if (sv1.y > sv2.y) std::swap(sv1, sv2);
}

bool Rasterizer::MiddleIsRight(const ScreenVertex& sv1, const ScreenVertex& sv2, const ScreenVertex& sv3)
{
    long cross = long(sv3.x - sv1.x) * (sv2.y - sv1.y) - long(sv3.y - sv1.y) * (sv2.x - sv1.x);
    return cross < 0;
}

void Rasterizer::WalkEdge(const ScreenVertex& sv1, const ScreenVertex& sv2, BordLigne* out)
{
    int distanceX = std::abs(sv2.x - sv1.x);
    int distanceY = sv2.y - sv1.y;
    if (distanceY == 0) return;

    int sensX = (sv2.x > sv1.x) ? 1 : -1;
    int err = distanceY / 2;
    int x = sv1.x;

    for (int y = sv1.y; y < sv2.y; ++y) {
        float t = float(y - sv1.y) / distanceY;
        if (y >= 0 && y < m_height) {
            out[y] = {
                x,
                Lerp(sv1.z, sv2.z, t),
                Lerp(sv1.r, sv2.r, t), Lerp(sv1.g, sv2.g, t), Lerp(sv1.b, sv2.b, t),
                Lerp(sv1.invW, sv2.invW, t) // <-- Prêt pour la perspective !
            };
        }
        for (err += distanceX; err > distanceY; err -= distanceY)
            x += sensX;
    }
}

void Rasterizer::FillRows(int y0, int y1, const BordLigne left[], const BordLigne right[])
{
    for (int y = std::max(y0, 0); y < std::min(y1, m_height); ++y) {
        const BordLigne& currentLeft = left[y];
        const BordLigne& currentRight = right[y];
        int width = currentRight.x - currentLeft.x;

        for (int x = std::max(currentLeft.x, 0); x < std::min(currentRight.x, m_width); ++x) {
            float t = (width == 0) ? 0.0f : float(x - currentLeft.x) / width;
            Plot(x, y,
                Lerp(currentLeft.z, currentRight.z, t),
                Lerp(currentLeft.r, currentRight.r, t),
                Lerp(currentLeft.g, currentRight.g, t),
                Lerp(currentLeft.b, currentRight.b, t));
        }
    }
}

void Rasterizer::FillRowFast(int y, const BordLigne& l, const BordLigne& r)
{
    int w = std::max(r.x - l.x, 1);
    float dz = (r.z - l.z) / w, dr = (r.r - l.r) / w;
    float dg = (r.g - l.g) / w, db = (r.b - l.b) / w;
    float z = l.z, cr = l.r, cg = l.g, cb = l.b;
    for (int x = l.x; x < r.x; x++) {
        if (x > 0 && x < m_width) Plot(x, y, z, cr, cg, cb);
        z += dz; cr += dr; cg += dg; cb += db;
    }
}

void Rasterizer::DrawTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
    ScreenVertex sv1 = ToScreen(v1);
    ScreenVertex sv2 = ToScreen(v2);
    ScreenVertex sv3 = ToScreen(v3);

    SortByY(sv1, sv2, sv3);

    // Allocation dynamique propre pour s'adapter à la taille de l'écran
    std::vector<BordLigne> longSide(m_height, BordLigne{ 0, 0, 0, 0, 0, 0 });
    std::vector<BordLigne> shortSide(m_height, BordLigne{ 0, 0, 0, 0, 0, 0 });

    WalkEdge(sv1, sv3, longSide.data());
    WalkEdge(sv1, sv2, shortSide.data());
    WalkEdge(sv2, sv3, shortSide.data());

    bool right = MiddleIsRight(sv1, sv2, sv3);
    FillRows(sv1.y, sv3.y, right ? longSide.data() : shortSide.data(), right ? shortSide.data() : longSide.data());
}

void Rasterizer::DrawSquare(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4)
{
    DrawTriangle(v1, v2, v3);
    DrawTriangle(v1, v3, v4);
}