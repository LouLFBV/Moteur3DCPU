#pragma once

#include "Types.h"
#include "Rasterizer.h"
#include "VertexProcessor.h"
#include "FragmentProcessor.h"
#include <functional>
#include <glm/glm.hpp>
#include <vector>

class GraphicsPipeline
{
public:
    using VertexShader = std::function<Vertex(const Vertex& vertex, const glm::mat4& mvp)>;
    using FragmentShader = std::function<uint32_t(const BordLigne&)>;

    GraphicsPipeline(int width, int height);

    void SetVertexShader(VertexShader newVertexShader);
    void SetFragmentShader(FragmentShader newFragmentShader);

    void DrawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, const glm::mat4& mvp);
    void DrawSquare(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const Vertex& vertex4, const glm::mat4& mvp);
    void DrawCube(const Cube& cube, const glm::mat4& mvp);

    void DrawSquareNDC(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const Vertex& vertex4);

    void Clear();
    void ClearZBuffer();
    void RenderScene();

    const void* GetPixels() const;
    int GetPitch() const;
    void ResetStats();
    const RenderStats& GetStats() const;

private:
    Rasterizer m_rasterizer;
    VertexProcessor m_vertexProcessor;
    FragmentProcessor m_fragmentProcessor;

    std::vector<DrawCommand> m_drawCommands;
};