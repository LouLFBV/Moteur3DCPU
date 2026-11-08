#pragma once
#include <array>
#include <functional>
#include <vector>
#include "Types.h"

using VertexShader = std::function<Vertex(const Vertex&, const glm::mat4&)>;

class VertexProcessor
{
public:
    void SetVertexShader(VertexShader shader);

    Vertex TransformVertex(const Vertex& vertex, const glm::mat4& mvp) const;

    bool IsTriangleVisibleClipSpace(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3) const;

    std::vector<std::array<Vertex, 3>> ProcessTransformedTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3) const;

    std::vector<std::array<Vertex, 3>> ProcessTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const glm::mat4& mvp) const;

    bool IsTriangleVisible(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const glm::mat4& mvp) const;

private:
    static constexpr int MaxClippedVertices = 10;
    static constexpr int PlaneCount = 6;

    VertexShader m_vertexShader;

    const std::array<ClipPlane, PlaneCount> m_planes = {
        ClipPlane::Near,
        ClipPlane::Far,
        ClipPlane::Left,
        ClipPlane::Right,
        ClipPlane::Bottom,
        ClipPlane::Top
    };

    float PlaneDist(const Vertex& vertex, ClipPlane plane) const;

    Vertex Mix(const Vertex& vertex1, const Vertex& vertex2, float t) const;

    Vertex CrossPlane(const Vertex& vertexFront, const Vertex& vertexBack, ClipPlane plane) const;

    int ClipAgainstPlane(ClipPlane plane, const Vertex* inVertices, Vertex* outVertices, int inCount) const;

    bool IsBackFace(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3) const;
};
