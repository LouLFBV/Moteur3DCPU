#include "GraphicsPipeline.h"
#include "Multithreading.h"
#include <algorithm>

GraphicsPipeline::GraphicsPipeline(int width, int height)
    : m_rasterizer(width, height)
{
    // Vertex Shader par défaut appliqué sur le VertexProcessor
    m_vertexProcessor.SetVertexShader([](const Vertex& vertex, const glm::mat4& mvp) {
        Vertex transformed = vertex;
        transformed.position = mvp * vertex.position;

        glm::mat3 normalMatrix = glm::mat3(mvp);
        glm::vec3 rotatedNormal = normalMatrix * vertex.normal;
        transformed.normal = glm::normalize(rotatedNormal);
        return transformed;
        });
}

void GraphicsPipeline::SetVertexShader(VertexShader newVertexShader)
{
    m_vertexProcessor.SetVertexShader(newVertexShader);
}

void GraphicsPipeline::SetFragmentShader(FragmentShader newFragmentShader)
{
    m_fragmentProcessor.SetFragmentShader(newFragmentShader);
}

void GraphicsPipeline::DrawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, const glm::mat4& mvp)
{
    RenderStats& stats = m_rasterizer.GetStats();

    stats.trianglesSubmitted++;

    Vertex tv0 = m_vertexProcessor.TransformVertex(v0, mvp);
    Vertex tv1 = m_vertexProcessor.TransformVertex(v1, mvp);
    Vertex tv2 = m_vertexProcessor.TransformVertex(v2, mvp);

    if (!m_vertexProcessor.IsTriangleVisibleClipSpace(tv0, tv1, tv2))
    {
        stats.trianglesFrustumCulled++;
        return;
    }

    stats.trianglesProcessed++;

    m_drawCommands.push_back({ tv0, tv1, tv2 });
}

void GraphicsPipeline::DrawSquare(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const Vertex& vertex4, const glm::mat4& mvp)
{
    DrawTriangle(vertex1, vertex2, vertex3, mvp);
    DrawTriangle(vertex1, vertex3, vertex4, mvp);
}

void GraphicsPipeline::DrawCube(const Cube& cube, const glm::mat4& mvp)
{
    DrawSquare(cube.faceDown[0], cube.faceDown[1], cube.faceDown[2], cube.faceDown[3], mvp);
    DrawSquare(cube.faceTop[0], cube.faceTop[1], cube.faceTop[2], cube.faceTop[3], mvp);
    DrawSquare(cube.faceFront[0], cube.faceFront[1], cube.faceFront[2], cube.faceFront[3], mvp);
    DrawSquare(cube.faceBack[0], cube.faceBack[1], cube.faceBack[2], cube.faceBack[3], mvp);
    DrawSquare(cube.faceLeft[0], cube.faceLeft[1], cube.faceLeft[2], cube.faceLeft[3], mvp);
    DrawSquare(cube.faceRight[0], cube.faceRight[1], cube.faceRight[2], cube.faceRight[3], mvp);
}

void GraphicsPipeline::DrawMesh(const std::vector<Vertex>& vertices, const glm::mat4& mvp)
{
    m_drawCommands.reserve(m_drawCommands.size() + vertices.size() / 3);

    for (size_t i = 0; i + 2 < vertices.size(); i += 3) {
        DrawTriangle( vertices[i], vertices[i + 1], vertices[i + 2], mvp );
    }
}

void GraphicsPipeline::DrawSquareNDC(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const Vertex& vertex4)
{
    glm::mat4 identity = glm::mat4(1.0f);
    DrawTriangle(vertex1, vertex2, vertex3, identity);
    DrawTriangle(vertex1, vertex3, vertex4, identity);
}

void GraphicsPipeline::Clear()
{
    m_rasterizer.Clear();
}

void GraphicsPipeline::ClearZBuffer()
{
    m_rasterizer.ClearZBuffer();
}

const void* GraphicsPipeline::GetPixels() const
{
    return m_rasterizer.GetPixels();
}

int GraphicsPipeline::GetPitch() const
{
    return m_rasterizer.GetPitch();
}

void GraphicsPipeline::ResetStats()
{
    m_rasterizer.ResetStats();
}

const RenderStats& GraphicsPipeline::GetStats() const
{
    return m_rasterizer.GetStats();
}

void GraphicsPipeline::SetAlphaBlendingEnabled(bool enabled)
{
    m_rasterizer.SetAlphaBlendingEnabled(enabled);
}

void GraphicsPipeline::SetDepthWriteEnabled(bool enabled)
{
    m_rasterizer.SetDepthWriteEnabled(enabled);
}

void GraphicsPipeline::SetDepthTestEnabled(bool enabled)
{
    m_rasterizer.SetDepthTestEnabled(enabled);
}


void GraphicsPipeline::RenderScene()
{
    m_rasterizer.GetStats().threadsUsed = Multithreading::GetInstance().GetNumThreads();

    int height = m_rasterizer.GetHeight();

    m_clippedTriangles.clear();
    m_clippedTriangles.reserve(m_drawCommands.size());

    for (const auto& cmd : m_drawCommands) {
        auto clippedTriangles = m_vertexProcessor.ProcessTransformedTriangle(
            cmd.v0,
            cmd.v1,
            cmd.v2
        );
        for (const auto& tri : clippedTriangles) {
            m_clippedTriangles.push_back(tri);
        }
    }



    int numJobs = Multithreading::GetInstance().GetNumJobs();

    Multithreading::GetInstance().DispatchTasks(numJobs, height, [this](int, int minY, int maxY) {

        auto pixelShaderCallback = [this](const BordLigne& pixel) {
            return m_fragmentProcessor.ProcessFragment(pixel);
            };

        for (const auto& tri : m_clippedTriangles) {
            m_rasterizer.RasterizeTriangleBox(
                tri[0], tri[1], tri[2],
                minY, maxY,
                pixelShaderCallback
            );
        }
        });

    Multithreading::GetInstance().WaitAll();
    m_drawCommands.clear();
}

void GraphicsPipeline::DrawText2D(int screenX, int screenY, const std::string& text, const Texture& fontAtlas, uint32_t color)
{
    int cursorX = screenX;
    int cursorY = screenY;


    constexpr int spacing = 1;
    constexpr int spaceAdvance = 10;
    constexpr int lineHeight = 22;

    // Vérification de la texture.
    if (fontAtlas.GetWidth() < 250 || fontAtlas.GetHeight() < 163) {
        return;
    }

    for (unsigned char c : text) {
        if (c == '\n') {
            cursorX = screenX;
            cursorY += lineHeight;
            continue;
        }

        if (c == '\r') {
            continue;
        }

        if (c == ' ') {
            cursorX += spaceAdvance;
            continue;
        }

        int srcX = 0;
        int srcY = 0;
        int charW = 0;
        int charH = 0;

        // Récupération de la cellule du caractère.
        if (!GetGlyphRect(
            static_cast<char>(c),
            fontAtlas.GetWidth(),
            fontAtlas.GetHeight(),
            srcX, srcY,
            charW, charH))
        {
            continue;
        }

        // Dessin du caractère dans le framebuffer.
        m_rasterizer.DrawSubImage2D(
            cursorX,
            cursorY,
            charW,
            charH,
            fontAtlas,
            srcX,
            srcY,
            color
        );

        // Passage au caractère suivant.
        cursorX += charW + spacing;
    }
}


bool GraphicsPipeline::GetGlyphRect(char c, int atlasWidth, int atlasHeight, int& outX, int& outY, int& outW, int& outH)
{
    struct Glyph
    {
        int x;
        int advance;
    };

    // Ligne 0 : ! " # $ % & ' ( ) * + , - . /
    static constexpr Glyph row0[] = {
        {4,12}, {16,18}, {34,22}, {56,18}, {74,25},
        {99,22}, {121,12}, {133,13}, {146,14}, {160,15},
        {175,21}, {196,13}, {209,16}, {225,11}, {236,14}
    };

    // Ligne 1 : 0 à 9
    static constexpr Glyph row1[] = {
        {4,18}, {22,12}, {34,18}, {52,18}, {70,18},
        {88,18}, {106,18}, {124,18}, {142,18}, {160,18}
    };

    // Ligne 2 : : ; < = > ? @
    static constexpr Glyph row2[] = {
        {4,12}, {16,11}, {27,16}, {43,15},
        {58,16}, {74,20}, {94,18}
    };

    // Ligne 3 : A à M
    static constexpr Glyph row3[] = {
        {4,18}, {22,18}, {40,18}, {58,18}, {76,18},
        {94,18}, {112,18}, {130,18}, {148,12}, {160,18},
        {178,18}, {196,18}, {214,18}
    };

    // Ligne 4 : N à Z
    static constexpr Glyph row4[] = {
        {4,21}, {25,18}, {43,18}, {61,20}, {81,18},
        {99,18}, {117,20}, {137,18}, {155,18}, {173,23},
        {196,18}, {214,20}, {234,16}
    };

    // Ligne 5 : [ \ ] ^ _ ` { | } ~
    static constexpr Glyph row5[] = {
        {4,14}, {18,16}, {34,13}, {47,20}, {67,16},
        {83,14}, {97,13}, {110,11}, {121,14}, {135,18}
    };

    // Ligne 6 : a à n
    static constexpr Glyph row6[] = {
        {4,18}, {22,18}, {40,16}, {56,18}, {74,18},
        {92,16}, {108,18}, {126,18}, {144,13}, {157,16},
        {173,18}, {191,14}, {205,22}, {227,18}
    };

    // Ligne 7 : o à z
    static constexpr Glyph row7[] = {
        {4,18}, {22,18}, {40,18}, {58,18}, {76,18},
        {94,16}, {110,18}, {128,18}, {146,23}, {169,18},
        {187,18}, {205,18}
    };

    unsigned char ascii = static_cast<unsigned char>(c);

    const Glyph* glyphs = nullptr;
    int count = 0;
    int index = -1;

    int rowY = 0;
    int rowHeight = 0;

    if (ascii >= 33 && ascii <= 47) {
        glyphs = row0;
        count = 15;
        index = ascii - 33;
        rowY = 0;
        rowHeight = 16;
    }
    else if (ascii >= 48 && ascii <= 57) {
        glyphs = row1;
        count = 10;
        index = ascii - 48;
        rowY = 20;
        rowHeight = 16;
    }
    else if (ascii >= 58 && ascii <= 64) {
        glyphs = row2;
        count = 7;
        index = ascii - 58;
        rowY = 40;
        rowHeight = 16;
    }
    else if (ascii >= 65 && ascii <= 77)
    {
        glyphs = row3;
        count = 13;
        index = ascii - 65;
        rowY = 60;
        rowHeight = 16;
    }
    else if (ascii >= 78 && ascii <= 90) {
        glyphs = row4;
        count = 13;
        index = ascii - 78;
        rowY = 80;
        rowHeight = 16;
    }
    else if (ascii >= 91 && ascii <= 96) {
        glyphs = row5;
        count = 10;
        index = ascii - 91;
        rowY = 100;
        rowHeight = 16;
    }
    else if (ascii >= 123 && ascii <= 126) {
        glyphs = row5;
        count = 10;
        index = 6 + (ascii - 123);
        rowY = 100;
        rowHeight = 16;
    }
    else if (ascii >= 97 && ascii <= 110) {
        glyphs = row6;
        count = 14;
        index = ascii - 97;
        rowY = 126;
        rowHeight = 17;
    }
    else if (ascii >= 111 && ascii <= 122) {
        glyphs = row7;
        count = 12;
        index = ascii - 111;
        rowY = 146;
        rowHeight = 17;
    }
    else {
        return false;
    }

    if (index < 0 || index >= count) {
        return false;
    }

    outX = glyphs[index].x;
    outY = rowY;
    outW = glyphs[index].advance;
    outH = rowHeight;

    return outX >= 0 && outY >= 0 && outX + outW <= atlasWidth && outY + outH <= atlasHeight;
}

void GraphicsPipeline::DrawFadeOverlay(float alpha, const glm::vec3& color)
{
    if (alpha <= 0.0f) {
        return;
    }

    uint32_t aInt = static_cast<uint32_t>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f);
    if (aInt == 0) {
        return;
    }

    uint32_t targetR = static_cast<uint32_t>(std::clamp(color.r, 0.0f, 1.0f) * 255.0f);
    uint32_t targetG = static_cast<uint32_t>(std::clamp(color.g, 0.0f, 1.0f) * 255.0f);
    uint32_t targetB = static_cast<uint32_t>(std::clamp(color.b, 0.0f, 1.0f) * 255.0f);

    uint32_t* pixels = static_cast<uint32_t*>(const_cast<void*>(m_rasterizer.GetPixels()));
    int totalPixels = m_rasterizer.GetWidth() * m_rasterizer.GetHeight();

    for (int i = 0; i < totalPixels; ++i) {
        uint32_t currentPixel = pixels[i];

        int r = (currentPixel >> 16) & 0xFF;
        int g = (currentPixel >> 8) & 0xFF;
        int b = currentPixel & 0xFF;

        r += (static_cast<int>(targetR) - r) * static_cast<int>(aInt) / 255;
        g += (static_cast<int>(targetG) - g) * static_cast<int>(aInt) / 255;
        b += (static_cast<int>(targetB) - b) * static_cast<int>(aInt) / 255;

        pixels[i] = 0xFF000000u |
            (static_cast<uint32_t>(r) << 16) |
            (static_cast<uint32_t>(g) << 8) |
            static_cast<uint32_t>(b);
    }
}