#include "VertexProcessor.h"
#include "MathUtils.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
    constexpr float Epsilon = 1e-6f;
}

void VertexProcessor::SetVertexShader(VertexShader shader)
{
    m_vertexShader = std::move(shader);
}

Vertex VertexProcessor::TransformVertex(const Vertex& vertex, const glm::mat4& mvp) const
{
    return m_vertexShader ? m_vertexShader(vertex, mvp) : vertex;
}

Vertex VertexProcessor::Mix(const Vertex& vertex1, const Vertex& vertex2, float t) const
{
    Vertex result{};

    result.position = glm::mix(vertex1.position, vertex2.position, t);

    result.r = glm::mix(vertex1.r, vertex2.r, t);
    result.g = glm::mix(vertex1.g, vertex2.g, t);
    result.b = glm::mix(vertex1.b, vertex2.b, t);
    result.alpha = glm::mix(vertex1.alpha, vertex2.alpha, t);

    result.uv = glm::mix(vertex1.uv, vertex2.uv, t);
    result.normal = glm::mix(vertex1.normal, vertex2.normal, t);
    result.worldPos = glm::mix(vertex1.worldPos, vertex2.worldPos, t);

    return result;
}

float VertexProcessor::PlaneDist(const Vertex& vertex, ClipPlane plane) const
{
    const glm::vec4& position = vertex.position;

    switch (plane) {
        case ClipPlane::Near:
            return position.w + position.z;
        case ClipPlane::Far: 
            return position.w - position.z;
        case ClipPlane::Left:
            return position.w + position.x;
        case ClipPlane::Right:
            return position.w - position.x;
        case ClipPlane::Bottom:
            return position.w + position.y;
        case ClipPlane::Top:
            return position.w - position.y;
    }

    return 0.0f;
}

Vertex VertexProcessor::CrossPlane(const Vertex& vertexFront, const Vertex& vertexBack, ClipPlane plane) const
{
    const float distanceFront = PlaneDist(vertexFront, plane);
    const float distanceBack = PlaneDist(vertexBack, plane);

    const float denominator = distanceFront - distanceBack;

    if (std::abs(denominator) < Epsilon) {
        return vertexFront;
    }

    const float t = std::clamp(distanceFront / denominator, 0.0f, 1.0f);

    return Mix(vertexFront, vertexBack, t);
}

int VertexProcessor::ClipAgainstPlane(ClipPlane plane, const Vertex* inVertices, Vertex* outVertices, int inCount) const
{
    if (inCount <= 0) {
        return 0;
    }

    int outputCount = 0;

    for (int i = 0; i < inCount; ++i) {
        const Vertex& start = inVertices[i];
        const Vertex& end = inVertices[(i + 1) % inCount]; // Pour revenir au premier point lorsque start correspond au dernier de la liste

        const float startDistance = PlaneDist(start, plane);
        const float endDistance = PlaneDist(end, plane);

        const bool startInside = startDistance > 0.0f;
        const bool endInside = endDistance > 0.0f;

        if (startInside) {
            outVertices[outputCount++] = start;
        }

        if (startInside != endInside) {
            outVertices[outputCount++] = CrossPlane(start, end, plane);
        }
    }

    return outputCount;
}

bool VertexProcessor::IsTriangleVisibleClipSpace(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3) const
{
    for (ClipPlane plane : m_planes) {
        const float d1 = PlaneDist(vertex1, plane);
        const float d2 = PlaneDist(vertex2, plane);
        const float d3 = PlaneDist(vertex3, plane);

        if (d1 < 0.0f && d2 < 0.0f && d3 < 0.0f) {
            return false;
        }
    }

    return true;
}

bool VertexProcessor::IsBackFace(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3) const
{
    const float w1 = vertex1.position.w;
    const float w2 = vertex2.position.w;
    const float w3 = vertex3.position.w;

    if (std::abs(w1) < Epsilon || std::abs(w2) < Epsilon || std::abs(w3) < Epsilon){
        return true;
    }

    const glm::vec2 p1 = glm::vec2(vertex1.position) / w1;
    const glm::vec2 p2 = glm::vec2(vertex2.position) / w2;
    const glm::vec2 p3 = glm::vec2(vertex3.position) / w3;

    const float area2 = (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);

    return area2 <= 0.0f;
}


std::vector<std::array<Vertex, 3>> VertexProcessor::ProcessTransformedTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3) const
{
    std::vector<std::array<Vertex, 3>> resultingTriangles;

    // Un triangle découpé par 6 plans peut produire
    // au maximum 9 sommets, donc 7 triangles.
    resultingTriangles.reserve(7);

    std::array<Vertex, MaxClippedVertices> poly1;
    std::array<Vertex, MaxClippedVertices> poly2;

    poly1[0] = vertex1;
    poly1[1] = vertex2;
    poly1[2] = vertex3;

    int vertexCount = 3;


    for (ClipPlane plane : m_planes) {
        vertexCount = ClipAgainstPlane(
            plane,
            poly1.data(),
            poly2.data(),
            vertexCount
        );

        if (vertexCount < 3) {
            return resultingTriangles;
        }

        std::copy_n(
            poly2.begin(),
            vertexCount,
            poly1.begin()
        );
    }

    for (int i = 1; i < vertexCount - 1; ++i) {
        const Vertex& a = poly1[0];
        const Vertex& b = poly1[i];
        const Vertex& c = poly1[i + 1];

        if (!IsBackFace(a, b, c)) {
            resultingTriangles.push_back({ a, b, c });
        }
    }

    return resultingTriangles;
}

std::vector<std::array<Vertex, 3>>VertexProcessor::ProcessTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const glm::mat4& mvp) const
{
    const Vertex tv1 = TransformVertex(vertex1, mvp);
    const Vertex tv2 = TransformVertex(vertex2, mvp);
    const Vertex tv3 = TransformVertex(vertex3, mvp);

    return ProcessTransformedTriangle(tv1, tv2, tv3);
}

bool VertexProcessor::IsTriangleVisible(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const glm::mat4& mvp) const
{
    const Vertex tv1 = TransformVertex(vertex1, mvp);
    const Vertex tv2 = TransformVertex(vertex2, mvp);
    const Vertex tv3 = TransformVertex(vertex3, mvp);

    return IsTriangleVisibleClipSpace(tv1, tv2, tv3);
}