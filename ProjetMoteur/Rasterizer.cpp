#include "Rasterizer.h"
#include "MathUtils.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <limits>

Rasterizer::Rasterizer(int width, int height) : m_width(width), m_height(height)
{
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument(
            "Rasterizer dimensions must be positive."
        );
    }

    const std::size_t pixelCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);

    // Les index utilisés dans le rasterizer sont des int.
    if (pixelCount > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error(
            "Rasterizer dimensions are too large."
        );
    }

    m_framebuffer.assign(pixelCount, 0xFF000000u);
    m_zbuffer.assign(pixelCount, 1.0f);
}

void Rasterizer::ResetStats()
{
    m_stats.trianglesSubmitted = 0;
    m_stats.trianglesFrustumCulled = 0;
    m_stats.trianglesProcessed = 0;

    m_stats.pixelsProcessed.store(0);

    m_stats.frameTimeMs = 0.0f;
    m_stats.threadsUsed = 0;
}

RenderStats& Rasterizer::GetStats()
{
    return m_stats;
}

const RenderStats& Rasterizer::GetStats() const
{
    return m_stats;
}

void Rasterizer::SetAlphaBlendingEnabled(bool enabled)
{
    m_alphaBlendingEnabled = enabled;
}

void Rasterizer::SetDepthTestEnabled(bool enabled)
{
    m_depthTestEnabled = enabled;
}

void Rasterizer::SetDepthWriteEnabled(bool enabled)
{
    m_depthWriteEnabled = enabled;
}

void Rasterizer::Clear()
{
    std::fill(
        m_framebuffer.begin(),
        m_framebuffer.end(),
        0xFF000000u
    );

    ClearZBuffer();
}

void Rasterizer::ClearZBuffer()
{
    std::fill(
        m_zbuffer.begin(),
        m_zbuffer.end(),
        1.0f
    );
}

uint32_t* Rasterizer::GetPixels()
{
    return m_framebuffer.data();
}

const void* Rasterizer::GetPixels() const
{
    return m_framebuffer.data();
}

int Rasterizer::GetPitch() const
{
    return m_width * static_cast<int>(sizeof(uint32_t));
}

int Rasterizer::GetWidth() const
{
    return m_width;
}

int Rasterizer::GetHeight() const
{
    return m_height;
}

bool Rasterizer::WritePixelColor(int index, uint32_t color)
{
    // Pas de blending : remplacement direct de la couleur.
    if (!m_alphaBlendingEnabled) {
        m_framebuffer[index] = color;
        return true;
    }

    const uint8_t sourceAlpha = (color >> 24) & 0xFF;

    // Pixel entièrement transparent.
    if (sourceAlpha == 0) {
        return false;
    }

    // Pixel entièrement opaque.
    if (sourceAlpha == 255) {
        m_framebuffer[index] = color;
        return true;
    }

    const uint8_t sourceRed = (color >> 16) & 0xFF;
    const uint8_t sourceGreen = (color >> 8) & 0xFF;
    const uint8_t sourceBlue = color & 0xFF;

    const uint32_t destinationColor = m_framebuffer[index];

    const uint8_t destinationRed = (destinationColor >> 16) & 0xFF;
    const uint8_t destinationGreen = (destinationColor >> 8) & 0xFF;
    const uint8_t destinationBlue = destinationColor & 0xFF;

    const float alpha = sourceAlpha / 255.0f;
    const float inverseAlpha = 1.0f - alpha;

    const uint8_t finalRed = static_cast<uint8_t>(
        sourceRed * alpha +
        destinationRed * inverseAlpha +
        0.5f
        );

    const uint8_t finalGreen = static_cast<uint8_t>(
        sourceGreen * alpha +
        destinationGreen * inverseAlpha +
        0.5f
        );

    const uint8_t finalBlue = static_cast<uint8_t>(
        sourceBlue * alpha +
        destinationBlue * inverseAlpha +
        0.5f
        );

    m_framebuffer[index] =
        (0xFFu << 24) |
        (static_cast<uint32_t>(finalRed) << 16) |
        (static_cast<uint32_t>(finalGreen) << 8) |
        static_cast<uint32_t>(finalBlue);

    return true;
}

ScreenVertex Rasterizer::ToScreen(const Vertex& vertex)
{
    constexpr float epsilon = 0.000001f;

    // Évite une division par zéro.
    if (std::abs(vertex.position.w) < epsilon) {
        return {};
    }

    const float inverseW = 1.0f / vertex.position.w;

    const float xNdc = vertex.position.x * inverseW;
    const float yNdc = vertex.position.y * inverseW;
    const float zNdc = vertex.position.z * inverseW;

    return
    {
        static_cast<int>(
            std::lround((xNdc + 1.0f) * 0.5f * m_width)
        ),

        static_cast<int>(
            std::lround((1.0f - yNdc) * 0.5f * m_height)
        ),

        zNdc,

        vertex.r,
        vertex.g,
        vertex.b,

        inverseW,

        vertex.uv.x,
        vertex.uv.y,

        vertex.normal.x,
        vertex.normal.y,
        vertex.normal.z,

        {
            vertex.worldPos.x,
            vertex.worldPos.y,
            vertex.worldPos.z
        },

        vertex.alpha
    };
}

void Rasterizer::Plot(int x, int y, float z, uint32_t color)
{
    m_stats.pixelsProcessed.fetch_add(
        1,
        std::memory_order_relaxed
    );

    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return;
    }

    const int index = x + y * m_width;

    // Test de profondeur.
    if (m_depthTestEnabled && z >= m_zbuffer[index]) {
        return;
    }

    const bool pixelWritten = WritePixelColor(index, color);

    // Les pixels transparents ne modifient pas le Z-buffer.
    if (pixelWritten && m_depthWriteEnabled) {
        m_zbuffer[index] = z;
    }
}

void Rasterizer::SortByY(ScreenVertex& screenVertex1, ScreenVertex& screenVertex2, ScreenVertex& screenVertex3)
{
    if (screenVertex1.y > screenVertex2.y) {
        std::swap(screenVertex1, screenVertex2);
    }

    if (screenVertex2.y > screenVertex3.y) {
        std::swap(screenVertex2, screenVertex3);
    }

    if (screenVertex1.y > screenVertex2.y) {
        std::swap(screenVertex1, screenVertex2);
    }
}

bool Rasterizer::MiddleIsRight(const ScreenVertex& screenVertex1, const ScreenVertex& screenVertex2, const ScreenVertex& screenVertex3)
{
    const long cross = static_cast<long>(screenVertex3.x - screenVertex1.x) * (screenVertex2.y - screenVertex1.y) - static_cast<long>(screenVertex3.y - screenVertex1.y) * (screenVertex2.x - screenVertex1.x);

    return cross < 0;
}

void Rasterizer::WalkEdge(const ScreenVertex& screenVertex1, const ScreenVertex& screenVertex2, BordLigne* out) const
{
    const int distanceX = std::abs(screenVertex2.x - screenVertex1.x);

    const int distanceY = screenVertex2.y - screenVertex1.y;

    if (distanceY == 0) {
        return;
    }

    const int directionX = (screenVertex2.x > screenVertex1.x) ? 1 : -1;

    int error = distanceY / 2;
    int x = screenVertex1.x;

    for (int y = screenVertex1.y; y < screenVertex2.y; ++y) {
        const float t = static_cast<float>(y - screenVertex1.y) / distanceY;

        if (y >= 0 && y < m_height) {
            out[y] =
            {
                x,

                Lerp(screenVertex1.z, screenVertex2.z, t),

                Lerp(screenVertex1.r, screenVertex2.r, t),
                Lerp(screenVertex1.g, screenVertex2.g, t),
                Lerp(screenVertex1.b, screenVertex2.b, t),

                Lerp(screenVertex1.invW, screenVertex2.invW, t),

                Lerp(screenVertex1.u, screenVertex2.u, t),
                Lerp(screenVertex1.v, screenVertex2.v, t),

                Lerp(screenVertex1.nx, screenVertex2.nx, t),
                Lerp(screenVertex1.ny, screenVertex2.ny, t),
                Lerp(screenVertex1.nz, screenVertex2.nz, t),

                {
                    Lerp(
                        screenVertex1.worldPos.x,
                        screenVertex2.worldPos.x,
                        t
                    ),

                    Lerp(
                        screenVertex1.worldPos.y,
                        screenVertex2.worldPos.y,
                        t
                    ),

                    Lerp(
                        screenVertex1.worldPos.z,
                        screenVertex2.worldPos.z,
                        t
                    )
                },

                Lerp(screenVertex1.alpha, screenVertex2.alpha, t)
            };
        }

        for (error += distanceX; error > distanceY; error -= distanceY){
            x += directionX;
        }
    }
}

void Rasterizer::FillRows(int y0, int y1, const BordLigne* left, const BordLigne* right, int tileMinY, int tileMaxY, const PixelShaderCallback& pixelShader)
{
    const int startY = std::max({
        y0,
        0,
        tileMinY
        });

    const int endY = std::min({
        y1,
        m_height,
        tileMaxY
        });

    for (int y = startY; y < endY; ++y) {
        FillRowFast(
            y,
            left[y],
            right[y],
            pixelShader
        );
    }
}

void Rasterizer::FillRowFast( int y, const BordLigne& left, const BordLigne& right, const PixelShaderCallback& pixelShader)
{
    const int width = std::max(right.x - left.x, 1);
    const float inverseWidth = 1.0f / width;

    const float deltaZ = (right.z - left.z) * inverseWidth;

    const float deltaR = (right.r - left.r) * inverseWidth;
    const float deltaG = (right.g - left.g) * inverseWidth;
    const float deltaB = (right.b - left.b) * inverseWidth;

    const float deltaU = (right.u - left.u) * inverseWidth;
    const float deltaV = (right.v - left.v) * inverseWidth;

    const float deltaNx = (right.nx - left.nx) * inverseWidth;
    const float deltaNy = (right.ny - left.ny) * inverseWidth;
    const float deltaNz = (right.nz - left.nz) * inverseWidth;

    const float deltaWorldX = (right.worldPos.x - left.worldPos.x) * inverseWidth;

    const float deltaWorldY = (right.worldPos.y - left.worldPos.y) * inverseWidth;

    const float deltaWorldZ = (right.worldPos.z - left.worldPos.z) * inverseWidth;

    const float deltaAlpha =  (right.alpha - left.alpha) * inverseWidth;

    BordLigne current = left;

    for (int x = left.x; x < right.x; ++x) {
        if (x >= 0 && x < m_width) {
            current.x = x;

            const uint32_t finalColor = pixelShader ? pixelShader(current) : 0xFFFFFFFFu;

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

float Rasterizer::Mix3(float value0, float value1, float value2, const float weights[3])
{
    return weights[0] * value0 +  weights[1] * value1 + weights[2] * value2;
}

float Rasterizer::EdgeFunction(const glm::vec2& pointA, const glm::vec2& pointB, const glm::vec2& pointC)
{
    return (pointC.x - pointA.x) * (pointB.y - pointA.y) - (pointC.y - pointA.y) * (pointB.x - pointA.x);
}

void Rasterizer::RasterizeTriangle(
    const Vertex& vertex0,
    const Vertex& vertex1,
    const Vertex& vertex2,
    int tileMinY,
    int tileMaxY,
    const PixelShaderCallback& pixelShader,
    std::vector<BordLigne>& longSide,
    std::vector<BordLigne>& shortSide
)
{
    ScreenVertex screenVertex1 = ToScreen(vertex0);
    ScreenVertex screenVertex2 = ToScreen(vertex1);
    ScreenVertex screenVertex3 = ToScreen(vertex2);

    SortByY(
        screenVertex1,
        screenVertex2,
        screenVertex3
    );

    WalkEdge(
        screenVertex1,
        screenVertex3,
        longSide.data()
    );

    WalkEdge(
        screenVertex1,
        screenVertex2,
        shortSide.data()
    );

    WalkEdge(
        screenVertex2,
        screenVertex3,
        shortSide.data()
    );

    const bool middleIsRight = MiddleIsRight(
        screenVertex1,
        screenVertex2,
        screenVertex3
    );

    FillRows(
        screenVertex1.y,
        screenVertex3.y,
        middleIsRight ? longSide.data() : shortSide.data(),
        middleIsRight ? shortSide.data() : longSide.data(),
        tileMinY,
        tileMaxY,
        pixelShader
    );
}

void Rasterizer::RasterizeTriangleBox(
    const Vertex& vertex0,
    const Vertex& vertex1,
    const Vertex& vertex2,
    int minY,
    int maxY,
    const PixelShaderCallback& pixelShaderCallback
)
{
    constexpr float epsilon = 0.000001f;


    if (std::abs(vertex0.position.w) < epsilon ||
        std::abs(vertex1.position.w) < epsilon ||
        std::abs(vertex2.position.w) < epsilon) {
        return;
    }

    const float inverseW0 = 1.0f / vertex0.position.w;
    const float inverseW1 = 1.0f / vertex1.position.w;
    const float inverseW2 = 1.0f / vertex2.position.w;


    const glm::vec2 pointA(
        (vertex0.position.x * inverseW0 + 1.0f) * 0.5f * m_width,
        (1.0f - vertex0.position.y * inverseW0) * 0.5f * m_height
    );

    const glm::vec2 pointB(
        (vertex1.position.x * inverseW1 + 1.0f) * 0.5f * m_width,
        (1.0f - vertex1.position.y * inverseW1) * 0.5f * m_height
    );

    const glm::vec2 pointC(
        (vertex2.position.x * inverseW2 + 1.0f) * 0.5f * m_width,
        (1.0f - vertex2.position.y * inverseW2) * 0.5f * m_height
    );

    const float area = EdgeFunction(
        pointA,
        pointB,
        pointC
    );

    // Ignore les triangles dégénérés.
    if (std::abs(area) <= 0.00001f)    {
        return;
    }

    const int minX = std::max(
        0,
        static_cast<int>(
            std::floor(std::min({
                pointA.x,
                pointB.x,
                pointC.x
                }))
            )
    );

    const int maxX = std::min(
        m_width - 1,
        static_cast<int>(
            std::ceil(std::max({
                pointA.x,
                pointB.x,
                pointC.x
                }))
            )
    );

    const int minYBox = std::max(
        0,
        static_cast<int>(
            std::floor(std::min({
                pointA.y,
                pointB.y,
                pointC.y
                }))
            )
    );

    const int maxYBox = std::min(
        m_height - 1,
        static_cast<int>(
            std::ceil(std::max({
                pointA.y,
                pointB.y,
                pointC.y
                }))
            )
    );

    // Aucun pixel dans la zone de rendu.
    if (minX > maxX ||
        minYBox > maxYBox ||
        maxYBox < minY ||
        minYBox >= maxY) {
        return;
    }

    const int startY = std::max(minYBox, minY);
    const int endY = std::min(maxYBox, maxY - 1);

    const auto isTopLeft = [](const glm::vec2& point1, const glm::vec2& point2) {
            const float deltaX = point2.x - point1.x;
            const float deltaY = point2.y - point1.y;

            return (deltaY < 0.0f) || (deltaY == 0.0f && deltaX > 0.0f);
        };

    const bool bias0 = isTopLeft(pointB, pointC);
    const bool bias1 = isTopLeft(pointC, pointA);
    const bool bias2 = isTopLeft(pointA, pointB);

    const float z0 = vertex0.position.z * inverseW0;
    const float z1 = vertex1.position.z * inverseW1;
    const float z2 = vertex2.position.z * inverseW2;

    for (int y = startY; y <= endY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            const glm::vec2 pixelPosition(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);

            const float edge0 = EdgeFunction(
                pointB,
                pointC,
                pixelPosition
            );

            const float edge1 = EdgeFunction(
                pointC,
                pointA,
                pixelPosition
            );

            const float edge2 = EdgeFunction(
                pointA,
                pointB,
                pixelPosition
            );

            if (edge0 < 0.0f || (edge0 == 0.0f && !bias0)) {
                continue;
            }

            if (edge1 < 0.0f || (edge1 == 0.0f && !bias1)) {
                continue;
            }

            if (edge2 < 0.0f || (edge2 == 0.0f && !bias2)) {
                continue;
            }

            m_stats.pixelsProcessed.fetch_add(1, std::memory_order_relaxed);

            const float weights[3] =
            {
                edge0 / area,
                edge1 / area,
                edge2 / area
            };

            const float z = Mix3(
                z0,
                z1,
                z2,
                weights
            );

            const int index = y * m_width + x;

            if (m_depthTestEnabled && z >= m_zbuffer[index]) {
                continue;
            }

            const float inverseWPixel = Mix3(
                inverseW0,
                inverseW1,
                inverseW2,
                weights
            );

            if (std::abs(inverseWPixel) < epsilon) {
                continue;
            }

            const float w = 1.0f / inverseWPixel;

            BordLigne pixel{};

            pixel.x = x;
            pixel.z = z;
            pixel.invW = inverseWPixel;

            pixel.u = Mix3(
                vertex0.uv.x * inverseW0,
                vertex1.uv.x * inverseW1,
                vertex2.uv.x * inverseW2,
                weights
            ) * w;

            pixel.v = Mix3(
                vertex0.uv.y * inverseW0,
                vertex1.uv.y * inverseW1,
                vertex2.uv.y * inverseW2,
                weights
            ) * w;

            pixel.r = Mix3(
                vertex0.r * inverseW0,
                vertex1.r * inverseW1,
                vertex2.r * inverseW2,
                weights
            ) * w;

            pixel.g = Mix3(
                vertex0.g * inverseW0,
                vertex1.g * inverseW1,
                vertex2.g * inverseW2,
                weights
            ) * w;

            pixel.b = Mix3(
                vertex0.b * inverseW0,
                vertex1.b * inverseW1,
                vertex2.b * inverseW2,
                weights
            ) * w;

            pixel.nx = Mix3(
                vertex0.normal.x * inverseW0,
                vertex1.normal.x * inverseW1,
                vertex2.normal.x * inverseW2,
                weights
            ) * w;

            pixel.ny = Mix3(
                vertex0.normal.y * inverseW0,
                vertex1.normal.y * inverseW1,
                vertex2.normal.y * inverseW2,
                weights
            ) * w;

            pixel.nz = Mix3(
                vertex0.normal.z * inverseW0,
                vertex1.normal.z * inverseW1,
                vertex2.normal.z * inverseW2,
                weights
            ) * w;

            pixel.worldPos.x = Mix3(
                vertex0.worldPos.x * inverseW0,
                vertex1.worldPos.x * inverseW1,
                vertex2.worldPos.x * inverseW2,
                weights
            ) * w;

            pixel.worldPos.y = Mix3(
                vertex0.worldPos.y * inverseW0,
                vertex1.worldPos.y * inverseW1,
                vertex2.worldPos.y * inverseW2,
                weights
            ) * w;

            pixel.worldPos.z = Mix3(
                vertex0.worldPos.z * inverseW0,
                vertex1.worldPos.z * inverseW1,
                vertex2.worldPos.z * inverseW2,
                weights
            ) * w;

            pixel.alpha = Mix3(
                vertex0.alpha * inverseW0,
                vertex1.alpha * inverseW1,
                vertex2.alpha * inverseW2,
                weights
            ) * w;

            const uint32_t finalColor =
                pixelShaderCallback
                ? pixelShaderCallback(pixel)
                : 0xFFFFFFFFu;

            const bool pixelWritten = WritePixelColor(
                index,
                finalColor
            );

            // Les fragments transparents ne modifient pas
            // le Z-buffer lorsqu'ils ne sont pas écrits.
            if (pixelWritten && m_depthWriteEnabled)
            {
                m_zbuffer[index] = z;
            }
        }
    }
}


void Rasterizer::DrawSubImage2D(
    int screenX,
    int screenY,
    int width,
    int height,
    const Texture& texture,
    int srcX,
    int srcY,
    uint32_t textColor
)
{
    const int textureWidth = texture.GetWidth();
    const int textureHeight = texture.GetHeight();

    const uint8_t textAlpha = (textColor >> 24) & 0xFF;
    const uint8_t textRed = (textColor >> 16) & 0xFF;
    const uint8_t textGreen = (textColor >> 8) & 0xFF;
    const uint8_t textBlue = textColor & 0xFF;

    const float textAlphaNormalized = textAlpha / 255.0f;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int destinationX = screenX + x;
            const int destinationY = screenY + y;

            // Vérification des coordonnées écran.
            if (destinationX < 0 || destinationX >= m_width || destinationY < 0 || destinationY >= m_height) {
                continue;
            }

            const int textureX = srcX + x;
            const int textureY = srcY + y;

            // Vérification des coordonnées texture.
            if (textureX < 0 || textureX >= textureWidth || textureY < 0 || textureY >= textureHeight) {
                continue;
            }

            const uint32_t sourceColor = texture.GetPixel(textureX, textureY);

            const uint8_t sourceAlpha = (sourceColor >> 24) & 0xFF;
            const uint8_t sourceRed = (sourceColor >> 16) & 0xFF;
            const uint8_t sourceGreen = (sourceColor >> 8) & 0xFF;
            const uint8_t sourceBlue = sourceColor & 0xFF;

            // Blanc = caractère, transparent = arrière-plan.
            const float luminance = (sourceRed + sourceGreen + sourceBlue) / (3.0f * 255.0f);

            const float coverage = luminance * (sourceAlpha / 255.0f);

            const float alpha = coverage * textAlphaNormalized;

            if (alpha <= 0.001f) {
                continue;
            }

            const int index = destinationY * m_width + destinationX;

            const uint32_t destinationColor = m_framebuffer[index];

            const uint8_t destinationRed = (destinationColor >> 16) & 0xFF;

            const uint8_t destinationGreen = (destinationColor >> 8) & 0xFF;

            const uint8_t destinationBlue = destinationColor & 0xFF;

            const auto blendChannel = [alpha](uint8_t source, uint8_t destination) {
                    return static_cast<uint8_t>(
                        source * alpha +
                        destination * (1.0f - alpha) +
                        0.5f
                        );
                };

            const uint8_t finalRed = blendChannel(
                textRed,
                destinationRed
            );

            const uint8_t finalGreen = blendChannel(
                textGreen,
                destinationGreen
            );

            const uint8_t finalBlue = blendChannel(
                textBlue,
                destinationBlue
            );

            m_framebuffer[index] =
                (0xFFu << 24) |
                (static_cast<uint32_t>(finalRed) << 16) |
                (static_cast<uint32_t>(finalGreen) << 8) |
                static_cast<uint32_t>(finalBlue);
        }
    }
}
