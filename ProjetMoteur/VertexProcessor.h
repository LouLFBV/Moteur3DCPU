#pragma once

#include <functional>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include "Types.h"
#include "MathUtils.h"

using VertexShader = std::function<Vertex(const Vertex&, const glm::mat4&)>;

class VertexProcessor {
public:
    void SetVertexShader(VertexShader shader);

    // Traite un triangle : Vertex Shader -> Clipping -> Backface Culling -> Retourne les triangles prêts
    std::vector<std::array<Vertex, 3>> ProcessTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, const glm::mat4& mvp);
    bool IsTriangleVisible(const Vertex& v1, const Vertex& v2, const Vertex& v3, const glm::mat4& mvp);

    Vertex TransformVertex(
        const Vertex& vertex,
        const glm::mat4& mvp
    ) const;

    bool IsTriangleVisibleClipSpace(
        const Vertex& v1,
        const Vertex& v2,
        const Vertex& v3
    ) const;

    std::vector<std::array<Vertex, 3>> ProcessTransformedTriangle(
        const Vertex& v1,
        const Vertex& v2,
        const Vertex& v3
    );

private:
    VertexShader m_vertexShader;
    ClipPlane m_planes[6] = {
        ClipPlane::Near, ClipPlane::Far,
        ClipPlane::Left, ClipPlane::Right,
        ClipPlane::Bottom, ClipPlane::Top
    };

    float PlaneDist(const Vertex& vertex, ClipPlane plane) const;
    Vertex CrossPlane(const Vertex& vertexFront, const Vertex& vertexBack, ClipPlane plane);
    int ClipAgainstPlane(ClipPlane plane, const Vertex* inVertices, Vertex* outVertices, int inCount);
    Vertex Mix(const Vertex& vertex1, const Vertex& vertex2, float t);
    bool IsBackFace(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3);
};