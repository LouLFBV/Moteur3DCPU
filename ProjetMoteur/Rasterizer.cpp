#include "Rasterizer.h"
#include "MathUtils.h"
#include <algorithm>
#include <cmath>
#include <thread>

Rasterizer::Rasterizer(int width, int height)
    : m_width(width), m_height(height)
{
    m_framebuffer.resize(width * height, 0xFF000000);
    m_zbuffer.resize(width * height, 1.0f);
}

void Rasterizer::ResetStats()
{
    m_stats.trianglesRasterized = 0;
    m_stats.pixelsProcessed.store(0);
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
                Lerp(screenVertex1.invW, screenVertex2.invW, t)
            };
        }
        for (err += distanceX; err > distanceY; err -= distanceY)
            x += sensX;
    }
}

void Rasterizer::FillRows(int y0, int y1, const BordLigne* left, const BordLigne* right, int tileMinY, int tileMaxY)
{
    int startY = std::max({ y0, 0, tileMinY });
    int endY = std::min({ y1, m_height, tileMaxY });

    for (int y = startY; y < endY; ++y) 
    {
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

void Rasterizer::FillRowFast(int y, const BordLigne& l, const BordLigne& r) // Pas encore utiliser 
{
    int w = std::max(r.x - l.x, 1);
    float dz = (r.z - l.z) / w;
    float dr = (r.r - l.r) / w;
    float dg = (r.g - l.g) / w;
    float db = (r.b - l.b) / w;
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

// Mesure la distance d'un sommet par rapport aux six plans du clipspace
float Rasterizer::PlaneDist(const Vertex& vertex, ClipPlane plane)
{
    switch (plane) 
    {
        case ClipPlane::Near:
            return vertex.position.w + vertex.position.z;
        case ClipPlane::Far:
            return vertex.position.w - vertex.position.z;
        case ClipPlane::Left:
            return vertex.position.w + vertex.position.x;
        case ClipPlane::Right:
            return vertex.position.w - vertex.position.x;
        case ClipPlane::Bottom:
            return vertex.position.w + vertex.position.y;
        case ClipPlane::Top:
            return vertex.position.w - vertex.position.y;
    }
    return 0.0f;
}

// Donne le vertex sur le plan de choisi, se trouvant entre les deux vertex en paramètre
Vertex Rasterizer::CrossPlane(const Vertex& vertexFront, const Vertex& vertexBack, ClipPlane plane)
{
    float distanceFront = PlaneDist(vertexFront, plane);
    float distanceBack = PlaneDist(vertexBack, plane);
    return Mix(vertexFront, vertexBack, distanceFront / (distanceFront - distanceBack));
}

int Rasterizer::ClipAgainstPlane(ClipPlane plane, const Vertex* inVertices, Vertex* outVertices, int inCount)
{
    if (inCount == 0) return 0;

    int newVertices = 0;
    for (int i = 0; i < inCount; i ++ ) {

        const Vertex& startPoint = inVertices[i];
        const Vertex& endPoint = inVertices[(i + 1) % inCount]; // pour obtenir 0 lorsque startPoint est à inCount

        bool startPointIn = PlaneDist(startPoint, plane) > 0;
        bool endPointIn = PlaneDist(endPoint, plane) > 0;

        if (startPointIn)
        {
            outVertices[newVertices++] = startPoint; // startPoint est dedans
        }

        if (startPointIn != endPointIn)
        {
            outVertices[newVertices++] = CrossPlane(startPoint, endPoint, plane); // l'arête traverse
        }
    }
    return newVertices; 
}


void Rasterizer::RasterizeTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, int tileMinY, int tileMaxY)
{
    m_stats.pixelsProcessed++;

    ScreenVertex sv1 = ToScreen(vertex1);
    ScreenVertex sv2 = ToScreen(vertex2);
    ScreenVertex sv3 = ToScreen(vertex3);

    m_stats.trianglesRasterized++;

    SortByY(sv1, sv2, sv3);

    // Allocation dynamique propre pour s'adapter à la taille de l'écran
    std::vector<BordLigne> longSide(m_height, BordLigne{ 0, 0, 0, 0, 0, 0 });
    std::vector<BordLigne> shortSide(m_height, BordLigne{ 0, 0, 0, 0, 0, 0 });

    WalkEdge(sv1, sv3, longSide.data());
    WalkEdge(sv1, sv2, shortSide.data());
    WalkEdge(sv2, sv3, shortSide.data());

    bool right = MiddleIsRight(sv1, sv2, sv3); 
    FillRows(sv1.y, sv3.y, right ? longSide.data() : shortSide.data(), right ? shortSide.data() : longSide.data(), tileMinY, tileMaxY);

}

void Rasterizer::DrawTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3)
{
    m_stats.trianglesProcessed++;

    Vertex poly1[10];
    Vertex poly2[10];

    poly1[0] = vertex1;
    poly1[1] = vertex2;
    poly1[2] = vertex3;

    int vertexCount = 3;

    for (int i = 0; i < 6; i++) 
    {
        vertexCount = ClipAgainstPlane(m_planes[i], poly1, poly2, vertexCount);

        if (vertexCount < 3) 
            return;

        for (int j = 0; j < vertexCount; j++) 
        {
            poly1[j] = poly2[j];
        }
    }

    for (int i = 1; i < vertexCount - 1; i++) 
    {
        //RasterizeTriangle(poly1[0], poly1[i], poly1[i + 1]);
        m_triangleQueue.push_back({ poly1[0], poly1[i], poly1[i + 1] });
    }
}

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


void Rasterizer::RenderScene()
{
    m_stats.threadsUsed = m_NUM_THREADS;
    m_stats.trianglesRasterized = m_triangleQueue.size(); 

    std::vector<std::thread> threads;
    int bandHeight = m_height / m_NUM_THREADS;

    for (int i = 0; i < m_NUM_THREADS; i++) 
    {
        int minY = i * bandHeight;
        int maxY = (i == m_NUM_THREADS - 1) ? m_height : (i + 1) * bandHeight;

        // On lance le thread (fonction lambda)
        threads.emplace_back([this, minY, maxY]() {
            for (const auto& tri : m_triangleQueue) {
                // Chaque thread traite TOUS les triangles, mais ne dessine que dans son couloir [minY, maxY]
                RasterizeTriangle(tri[0], tri[1], tri[2], minY, maxY);
            }
            });
    }

    for (auto& t : threads) {
        t.join();
    }

    m_triangleQueue.clear();
}