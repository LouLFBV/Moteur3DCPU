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
    m_stats.trianglesSubmitted = 0;
    m_stats.trianglesFrustumCulled = 0;
    m_stats.trianglesProcessed = 0;

    m_stats.pixelsProcessed.store(0);

    m_stats.frameTimeMs = 0.0f;
    m_stats.threadsUsed = 0;
}

const RenderStats& Rasterizer::GetStats() const
{
    return m_stats;
}

void Rasterizer::SetDepthTestEnabled(bool enabled)
{
    m_depthTestEnabled = enabled;
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

bool Rasterizer::WritePixelColor(int index, uint32_t color)
{
    if (!m_alphaBlendingEnabled)
    {
        m_framebuffer[index] = color;
        return true;
    }

    uint8_t srcA = (color >> 24) & 0xFF;

    if (srcA == 0)
        return false;

    if (srcA == 255)
    {
        m_framebuffer[index] = color;
        return true;
    }

    uint8_t srcR = (color >> 16) & 0xFF;
    uint8_t srcG = (color >> 8) & 0xFF;
    uint8_t srcB = color & 0xFF;

    uint32_t destination = m_framebuffer[index];

    uint8_t dstR = (destination >> 16) & 0xFF;
    uint8_t dstG = (destination >> 8) & 0xFF;
    uint8_t dstB = destination & 0xFF;

    float alpha = srcA / 255.0f;
    float inverseAlpha = 1.0f - alpha;

    uint8_t finalR = static_cast<uint8_t>(
        srcR * alpha + dstR * inverseAlpha + 0.5f
        );

    uint8_t finalG = static_cast<uint8_t>(
        srcG * alpha + dstG * inverseAlpha + 0.5f
        );

    uint8_t finalB = static_cast<uint8_t>(
        srcB * alpha + dstB * inverseAlpha + 0.5f
        );

    m_framebuffer[index] =
        (0xFFu << 24) |
        (static_cast<uint32_t>(finalR) << 16) |
        (static_cast<uint32_t>(finalG) << 8) |
        finalB;

    return true;
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
        },
        vertex.alpha
    };
}

bool Rasterizer::Barycentric(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& p, float l[3])
{
    float area = EdgeFunction(a, b, c);
    if (std::abs(area) < 0.00001f) return false; 

    l[0] = EdgeFunction(b, c, p) / area;
    l[1] = EdgeFunction(c, a, p) / area;
    l[2] = EdgeFunction(a, b, p) / area;
    return true;
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
                },
                Lerp(screenVertex1.alpha, screenVertex2.alpha, t)
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

    float deltaAlpha = (right.alpha - left.alpha) * invWidth;

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

        current.alpha += deltaAlpha;
    }
}

Box Rasterizer::BoundingBox(
    const Vertex& a,
    const Vertex& b,
    const Vertex& c,
    int width,
    int height)
{
    constexpr float epsilon = 0.0000001f;

    if (std::abs(a.position.w) < epsilon ||
        std::abs(b.position.w) < epsilon ||
        std::abs(c.position.w) < epsilon)
    {
        return { 0, 0, -1, -1 };
    }

    auto ToScreenPosition = [width, height](const Vertex& vertex)
        {
            float invW = 1.0f / vertex.position.w;

            return glm::vec2(
                (vertex.position.x * invW + 1.0f) * 0.5f * width,
                (1.0f - vertex.position.y * invW) * 0.5f * height
            );
        };

    glm::vec2 A = ToScreenPosition(a);
    glm::vec2 B = ToScreenPosition(b);
    glm::vec2 C = ToScreenPosition(c);

    int minX = std::max(
        0,
        static_cast<int>(std::floor(std::min({ A.x, B.x, C.x })))
    );

    int minY = std::max(
        0,
        static_cast<int>(std::floor(std::min({ A.y, B.y, C.y })))
    );

    int maxX = std::min(
        width - 1,
        static_cast<int>(std::ceil(std::max({ A.x, B.x, C.x })))
    );

    int maxY = std::min(
        height - 1,
        static_cast<int>(std::ceil(std::max({ A.y, B.y, C.y })))
    );

    return { minX, minY, maxX, maxY };
}

float Rasterizer::Mix3(float a, float b, float c, const float l[3])
{
    return l[0] * a + l[1] * b + l[2] * c;
}

float Rasterizer::EdgeFunction(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
{
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

void Rasterizer::RasterizeTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3,
    int tileMinY, int tileMaxY,
    const PixelShaderCallback& pixelShader,
    std::vector<BordLigne>& longSide,
    std::vector<BordLigne>& shortSide)
{

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


void Rasterizer::RasterizeTriangleBox(const Vertex& v0, const Vertex& v1, const Vertex& v2, int minY, int maxY, const PixelShaderCallback& pixelShaderCallback)
{
    //m_stats.trianglesRasterized++;

    float invW0 = 1.0f / v0.position.w;
    float invW1 = 1.0f / v1.position.w;
    float invW2 = 1.0f / v2.position.w;

    glm::vec2 A((v0.position.x * invW0 + 1.0f) * 0.5f * m_width, (1.0f - v0.position.y * invW0) * 0.5f * m_height);
    glm::vec2 B((v1.position.x * invW1 + 1.0f) * 0.5f * m_width, (1.0f - v1.position.y * invW1) * 0.5f * m_height);
    glm::vec2 C((v2.position.x * invW2 + 1.0f) * 0.5f * m_width, (1.0f - v2.position.y * invW2) * 0.5f * m_height);

    float area = EdgeFunction(A, B, C);

    // 1. BACKFACE CULLING : Ignore les faces cachées. Élimine instantanément le Z-Fighting du cube !
    // ⚠️ ATTENTION : Si ton modèle devient invisible, change <= en >= (selon l'ordre de tes sommets OBJ)
    if (std::abs(area) <= 0.00001f) return;

    int minX_box = std::max(0, static_cast<int>(std::floor(std::min({ A.x, B.x, C.x }))));
    int maxX_box = std::min(m_width - 1, static_cast<int>(std::ceil(std::max({ A.x, B.x, C.x }))));
    int minY_box = std::max(0, static_cast<int>(std::floor(std::min({ A.y, B.y, C.y }))));
    int maxY_box = std::min(m_height - 1, static_cast<int>(std::ceil(std::max({ A.y, B.y, C.y }))));

    if (maxY_box < minY || minY_box >= maxY) return;

    int startY = std::max(minY_box, minY);
    int endY = std::min(maxY_box, maxY - 1);

    // 2. TIE-BREAKING RULE (Règle Top-Left) : Détermine à qui appartient un pixel sur une bordure parfaite
    auto isTopLeft = [](const glm::vec2& p1, const glm::vec2& p2) {
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;
        return (dy < 0.0f) || (dy == 0.0f && dx > 0.0f);
        };
    bool bias0 = isTopLeft(B, C);
    bool bias1 = isTopLeft(C, A);
    bool bias2 = isTopLeft(A, B);

    // Pré-calcul du Z (qui n'a pas besoin de correction de perspective)
    float z0 = v0.position.z * invW0, z1 = v1.position.z * invW1, z2 = v2.position.z * invW2;

    for (int y = startY; y <= endY; y++) {
        for (int x = minX_box; x <= maxX_box; x++) {
            glm::vec2 p(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);

            // On utilise EdgeFunction au lieu des divisions pour tester l'inclusion parfaite
            float w0 = EdgeFunction(B, C, p);
            float w1 = EdgeFunction(C, A, p);
            float w2 = EdgeFunction(A, B, p);

            // Application stricte de la règle (bouche 100% des trous)
            if (w0 < 0 || (w0 == 0.0f && !bias0)) continue;
            if (w1 < 0 || (w1 == 0.0f && !bias1)) continue;
            if (w2 < 0 || (w2 == 0.0f && !bias2)) continue;

            m_stats.pixelsProcessed.fetch_add(
                1,
                std::memory_order_relaxed
            );

            // Normalisation pour l'interpolation
            float l[3] = { w0 / area, w1 / area, w2 / area };
            float z = Mix3(z0, z1, z2, l);
            int index = y * m_width + x;

            if (!m_depthTestEnabled || z < m_zbuffer[index]) {

                // 3. CORRECTION DE PERSPECTIVE : Répare les textures et couleurs déformées
                float invW_pixel = Mix3(invW0, invW1, invW2, l);
                float w = 1.0f / invW_pixel;

                BordLigne pixel{};

                // Coordonnées du pixel sur l'écran
                pixel.x = x;

                // Profondeur
                pixel.z = z;

                // Inverse de W
                pixel.invW = invW_pixel;

                // Application du ratio W sur les UVs
                pixel.u = Mix3(v0.uv.x * invW0, v1.uv.x * invW1, v2.uv.x * invW2, l) * w;
                pixel.v = Mix3(v0.uv.y * invW0, v1.uv.y * invW1, v2.uv.y * invW2, l) * w;

                // Application du ratio W sur les Couleurs
                pixel.r = Mix3(v0.r * invW0, v1.r * invW1, v2.r * invW2, l) * w;
                pixel.g = Mix3(v0.g * invW0, v1.g * invW1, v2.g * invW2, l) * w;
                pixel.b = Mix3(v0.b * invW0, v1.b * invW1, v2.b * invW2, l) * w;

                // Application du ratio W sur les Normales et Positions
                pixel.nx = Mix3(v0.normal.x * invW0, v1.normal.x * invW1, v2.normal.x * invW2, l) * w;
                pixel.ny = Mix3(v0.normal.y * invW0, v1.normal.y * invW1, v2.normal.y * invW2, l) * w;
                pixel.nz = Mix3(v0.normal.z * invW0, v1.normal.z * invW1, v2.normal.z * invW2, l) * w;

                pixel.worldPos.x = Mix3(v0.worldPos.x * invW0, v1.worldPos.x * invW1, v2.worldPos.x * invW2, l) * w;
                pixel.worldPos.y = Mix3(v0.worldPos.y * invW0, v1.worldPos.y * invW1, v2.worldPos.y * invW2, l) * w;
                pixel.worldPos.z = Mix3(v0.worldPos.z * invW0, v1.worldPos.z * invW1, v2.worldPos.z * invW2, l) * w;

                // Interpolation de l'alpha.
                pixel.alpha = Mix3(
                    v0.alpha * invW0,
                    v1.alpha * invW1,
                    v2.alpha * invW2,
                    l
                ) * w;

                // Calcul de la couleur finale du fragment.
                uint32_t finalColor = pixelShaderCallback(pixel);

                // Mélange de la couleur avec le framebuffer.
                bool pixelWritten = WritePixelColor(index, finalColor);

                // Les fragments transparents ne modifient pas le Z-Buffer
                // si l'écriture de profondeur est désactivée.
                if (pixelWritten && m_depthWriteEnabled)
                {
                    m_zbuffer[index] = z;
                }
            }
        }
    }
}


void Rasterizer::DrawSubImage2D(
    int screenX, int screenY,
    int width, int height,
    const Texture& texture,
    int srcX, int srcY,
    uint32_t textColor)
{
    int texW = texture.GetWidth();
    int texH = texture.GetHeight();

    uint8_t textA = (textColor >> 24) & 0xFF;
    uint8_t textR = (textColor >> 16) & 0xFF;
    uint8_t textG = (textColor >> 8) & 0xFF;
    uint8_t textB = textColor & 0xFF;

    float textAlpha = textA / 255.0f;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int destX = screenX + x;
            int destY = screenY + y;

            if (destX < 0 || destX >= m_width ||
                destY < 0 || destY >= m_height)
                continue;

            int texX = srcX + x;
            int texY = srcY + y;

            if (texX < 0 || texX >= texW ||
                texY < 0 || texY >= texH)
                continue;

            uint32_t srcColor = texture.GetPixel(texX, texY);

            uint8_t a = (srcColor >> 24) & 0xFF;
            uint8_t r = (srcColor >> 16) & 0xFF;
            uint8_t g = (srcColor >> 8) & 0xFF;
            uint8_t b = srcColor & 0xFF;

            // Blanc = caractère, transparent = arrière-plan.
            float luminance = (r + g + b) / (3.0f * 255.0f);
            float coverage = luminance * (a / 255.0f);
            float alpha = coverage * textAlpha;

            if (alpha <= 0.001f)
                continue;

            int index = destY * m_width + destX;

            uint32_t dstColor = m_framebuffer[index];

            uint8_t dstR = (dstColor >> 16) & 0xFF;
            uint8_t dstG = (dstColor >> 8) & 0xFF;
            uint8_t dstB = dstColor & 0xFF;

            auto Blend = [alpha](uint8_t src, uint8_t dst)
                {
                    return static_cast<uint8_t>(
                        src * alpha + dst * (1.0f - alpha) + 0.5f
                        );
                };

            uint8_t finalR = Blend(textR, dstR);
            uint8_t finalG = Blend(textG, dstG);
            uint8_t finalB = Blend(textB, dstB);

            m_framebuffer[index] =
                (0xFFu << 24) |
                (finalR << 16) |
                (finalG << 8) |
                finalB;
        }
    }
}

void Rasterizer::SetAlphaBlendingEnabled(bool enabled)
{
    m_alphaBlendingEnabled = enabled;
}

void Rasterizer::SetDepthWriteEnabled(bool enabled)
{
    m_depthWriteEnabled = enabled;
}
