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

void Rasterizer::ResetStats()
{
    m_stats.trianglesRasterized = 0;
    m_stats.pixelsProcessed = 0;
    m_stats.frameTimeMs = 0.0f;
    // On laisse threadsUsed à 1 pour le moment
}

const RenderStats& Rasterizer::GetStats() const
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
        inverseW 
    };
}

void Rasterizer::Plot(int x, int y, float z, float r, float g, float b)
{
    m_stats.pixelsProcessed++;

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

void Rasterizer::WalkEdge(const ScreenVertex& screenVertex1, const ScreenVertex& screenVertex2, BordLigne* out)
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
                Lerp(screenVertex1.invW, screenVertex2.invW, t)
            };
        }
        for (err += distanceX; err > distanceY; err -= distanceY)
            x += sensX;
    }
}

void Rasterizer::FillRows(int y0, int y1, const BordLigne* left, const BordLigne* right)
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

Vertex Rasterizer::Mix(const Vertex& vertex1, const Vertex& vertex2, float t)
{
    return {
        { Lerp(vertex1.position.x, vertex2.position.x, t),
        Lerp(vertex1.position.y, vertex2.position.y, t),
        Lerp(vertex1.position.z, vertex2.position.z, t),
        Lerp(vertex1.position.w, vertex2.position.w, t) },

        Lerp(vertex1.r, vertex2.r, t), Lerp(vertex1.g, vertex2.g, t), Lerp(vertex1.b, vertex2.b, t)
    };
}

float Rasterizer::NearDist(const Vertex& vertex)
{
    return vertex.position.w + vertex.position.z;
}

Vertex Rasterizer::CrossNear(const Vertex& p, const Vertex& q)
{
    float dp = NearDist(p);
    float dq = NearDist(q);
    return Mix(p, q, dp / (dp - dq));
}

// poly reçoit 0, 3 ou 4 sommets; la fonction renvoie leur nombre
int Rasterizer::ClipNear(const Vertex in[3], Vertex poly[4])
{
    int n = 0;
    for (int i = 0; i < 3; i ++ ) {
        const Vertex& p = in[i];
        const Vertex& q = in[(i + 1) % 3];
        bool pIn = NearDist(p) > 0, qIn = NearDist(q) > 0;
        if (pIn) poly[n ++ ] = p; // p est dedans
            if (pIn != qIn) poly[n ++ ] = CrossNear(p, q); // l'arête traverse
    }
    return n; // si n = 4 : triangles(0, 1, 2) puis(0, 2, 3)
}

// Change juste le nom de cette fonction existante
void Rasterizer::RasterizeTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3)
{
    m_stats.pixelsProcessed++;

    ScreenVertex sv1 = ToScreen(vertex1);
    ScreenVertex sv2 = ToScreen(vertex2);
    ScreenVertex sv3 = ToScreen(vertex3);

    // Bientôt, on fera le "Clipping" ici. Si un triangle est en dehors 
    // de l'écran, on fera "return;" et il ne sera jamais rasterisé.

    // S'il arrive jusqu'ici, c'est qu'on le dessine vraiment sur l'écran
    m_stats.trianglesRasterized++;

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

void Rasterizer::DrawTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3)
{
    m_stats.trianglesProcessed++;

    // On prépare le tableau d'entrée et le tableau de sortie
    Vertex in[3] = { vertex1, vertex2, vertex3 };
    Vertex poly[4];

    // On laisse la fonction du cours faire toute la magie géométrique
    int n = ClipNear(in, poly);

    // Selon le nombre de sommets générés, on dessine le résultat
    if (n == 3) {
        // Le triangle n'a pas été coupé, ou a été réduit à 3 sommets
        RasterizeTriangle(poly[0], poly[1], poly[2]);
    }
    else if (n == 4) {
        // Le triangle a été coupé en quadrilatère, on le sépare en 2 triangles
        RasterizeTriangle(poly[0], poly[1], poly[2]);
        RasterizeTriangle(poly[0], poly[2], poly[3]);
    }
    // Si n == 0, le triangle est derrière la caméra, on ne fait rien !
}

//void Rasterizer::DrawTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3)
//{
//    m_stats.pixelsProcessed++;
//
//    ScreenVertex sv1 = ToScreen(vertex1);
//    ScreenVertex sv2 = ToScreen(vertex2);
//    ScreenVertex sv3 = ToScreen(vertex3);
//
//    // Bientôt, on fera le "Clipping" ici. Si un triangle est en dehors 
//    // de l'écran, on fera "return;" et il ne sera jamais rasterisé.
//
//    // S'il arrive jusqu'ici, c'est qu'on le dessine vraiment sur l'écran
//    m_stats.trianglesRasterized++;
//
//    SortByY(sv1, sv2, sv3);
//
//    // Allocation dynamique propre pour s'adapter à la taille de l'écran
//    std::vector<BordLigne> longSide(m_height, BordLigne{ 0, 0, 0, 0, 0, 0 });
//    std::vector<BordLigne> shortSide(m_height, BordLigne{ 0, 0, 0, 0, 0, 0 });
//
//    WalkEdge(sv1, sv3, longSide.data());
//    WalkEdge(sv1, sv2, shortSide.data());
//    WalkEdge(sv2, sv3, shortSide.data());
//
//    bool right = MiddleIsRight(sv1, sv2, sv3);
//    FillRows(sv1.y, sv3.y, right ? longSide.data() : shortSide.data(), right ? shortSide.data() : longSide.data());
//}

void Rasterizer::DrawSquare(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const Vertex& vertex4)
{
    DrawTriangle(vertex1, vertex2, vertex3);
    DrawTriangle(vertex1, vertex3, vertex4);
}

void Rasterizer::DrawCube(const Cube& cube)
{
    DrawSquare(cube.faceDown[0], cube.faceDown[1], cube.faceDown[2], cube.faceDown[3]);
    DrawSquare(cube.faceTop[0], cube.faceTop[1], cube.faceTop[2], cube.faceTop[3]);
    DrawSquare(cube.faceFront[0], cube.faceFront[1], cube.faceFront[2], cube.faceFront[3]);
    DrawSquare(cube.faceBack[0], cube.faceBack[1], cube.faceBack[2], cube.faceBack[3]);
    DrawSquare(cube.faceLeft[0], cube.faceLeft[1], cube.faceLeft[2], cube.faceLeft[3]);
    DrawSquare(cube.faceRight[0], cube.faceRight[1], cube.faceRight[2], cube.faceRight[3]);
}