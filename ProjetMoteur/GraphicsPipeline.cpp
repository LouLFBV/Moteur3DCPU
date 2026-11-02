#include "GraphicsPipeline.h"
#include "Multithreading.h"
#include "VertexProcessor.h"

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

void GraphicsPipeline::DrawTriangle(
    const Vertex& v0,
    const Vertex& v1,
    const Vertex& v2,
    const glm::mat4& mvp)
{
    RenderStats& stats = m_rasterizer.GetStats();

    // 1. Triangle soumis au pipeline.
    stats.trianglesSubmitted++;

    // 2. Transformation des sommets (une seule fois).
    Vertex tv0 = m_vertexProcessor.TransformVertex(v0, mvp);
    Vertex tv1 = m_vertexProcessor.TransformVertex(v1, mvp);
    Vertex tv2 = m_vertexProcessor.TransformVertex(v2, mvp);

    // 3. Frustum Culling.
    if (!m_vertexProcessor.IsTriangleVisibleClipSpace(tv0, tv1, tv2))
    {
        stats.trianglesFrustumCulled++;
        return;
    }

    // 4. Triangle conservé.
    stats.trianglesProcessed++;

    // 5. Stockage des sommets déjà transformés.
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


void GraphicsPipeline::RenderScene()
{
    m_rasterizer.GetStats().threadsUsed = Multithreading::GetInstance().GetNumThreads();
    m_rasterizer.GetStats().trianglesProcessed = m_drawCommands.size();

    int height = m_rasterizer.GetHeight();

    m_clippedTriangles.clear();

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

    Multithreading::GetInstance().DispatchTasks(numJobs, height, [this](int taskId, int minY, int maxY) {

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