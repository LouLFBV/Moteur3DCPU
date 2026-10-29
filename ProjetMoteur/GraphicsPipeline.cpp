#include "GraphicsPipeline.h"
#include "Multithreading.h"

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
    m_drawCommands.push_back({ v0, v1, v2, mvp });
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

    // Distribution aux workers
    Multithreading::GetInstance().DispatchTasks(height, [this, height](int taskId, int minY, int maxY) {

        // Buffers temporaires propres à chaque worker
        std::vector<BordLigne> taskLongSide(height);
        std::vector<BordLigne> taskShortSide(height);

        // Callback générique vers le FragmentProcessor
        auto pixelShaderCallback = [this](const BordLigne& pixel) {
            return m_fragmentProcessor.ProcessFragment(pixel);
            };

        for (const auto& cmd : m_drawCommands) {

            // 1. Passage dans le VertexProcessor (Vertex Shader + Clipping + Culling)
            auto clippedTriangles = m_vertexProcessor.ProcessTriangle(cmd.v0, cmd.v1, cmd.v2, cmd.mvp);

            // 2. Passage au Rasterizer avec le callback du FragmentProcessor
            for (const auto& tri : clippedTriangles) {
                m_rasterizer.RasterizeTriangle(
                    tri[0], tri[1], tri[2],
                    minY, maxY,
                    pixelShaderCallback,
                    taskLongSide, taskShortSide
                );
            }
        }
        });

    Multithreading::GetInstance().WaitAll();
    m_drawCommands.clear();
}