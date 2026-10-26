#include "GraphicsPipeline.h"

GraphicsPipeline::GraphicsPipeline(int width, int height)
    : m_rasterizer(width, height)
{
    // Vertex Shader par défaut : Application classique de la matrice MVP
    m_vertexShader = [](const Vertex& vertex, const glm::mat4& mvp) {
        Vertex transformed = vertex;
        transformed.position = mvp * vertex.position;
        return transformed;
        };
}

void GraphicsPipeline::SetVertexShader(VertexShader newVertexShader)
{
    m_vertexShader = newVertexShader;
}

void GraphicsPipeline::SetFragmentShader(FragmentShader newFragmentShader)
{
    m_rasterizer.SetPixelShader(newFragmentShader);
}

void GraphicsPipeline::DrawTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const glm::mat4& mvp)
{
    // ÉTAPE DU VERTEX SHADER : Transformation des sommets
    Vertex tv1 = m_vertexShader(vertex1, mvp);
    Vertex tv2 = m_vertexShader(vertex2, mvp);
    Vertex tv3 = m_vertexShader(vertex3, mvp);

    // Passage au rastériseur (Clipping, Backface Culling, Rasterisation)
    m_rasterizer.DrawTriangle(tv1, tv2, tv3);
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
    m_rasterizer.DrawTriangle(vertex1, vertex2, vertex3);
    m_rasterizer.DrawTriangle(vertex1, vertex3, vertex4);
}

void GraphicsPipeline::Clear() 
{
    m_rasterizer.Clear(); 
}

void GraphicsPipeline::ClearZBuffer() 
{
    m_rasterizer.ClearZBuffer(); 
}

void GraphicsPipeline::RenderScene() 
{
    m_rasterizer.RenderScene(); 
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