#include "VertexProcessor.h"

Vertex VertexProcessor::Mix(const Vertex& vertex1, const Vertex& vertex2, float t)
{
    return {
        {
            Lerp(vertex1.position.x, vertex2.position.x, t),
            Lerp(vertex1.position.y, vertex2.position.y, t),
            Lerp(vertex1.position.z, vertex2.position.z, t),
            Lerp(vertex1.position.w, vertex2.position.w, t)
        },
        Lerp(vertex1.r, vertex2.r, t), Lerp(vertex1.g, vertex2.g, t), Lerp(vertex1.b, vertex2.b, t),
        { Lerp(vertex1.uv.x, vertex2.uv.x, t), Lerp(vertex1.uv.y, vertex2.uv.y, t) },
        { Lerp(vertex1.normal.x, vertex2.normal.x, t), Lerp(vertex1.normal.y, vertex2.normal.y, t), Lerp(vertex1.normal.z, vertex2.normal.z, t) },
        { Lerp(vertex1.worldPos.x, vertex2.worldPos.x, t), Lerp(vertex1.worldPos.y, vertex2.worldPos.y, t), Lerp(vertex1.worldPos.z, vertex2.worldPos.z, t) },
        Lerp(vertex1.alpha, vertex2.alpha, t)
    };
}

void VertexProcessor::SetVertexShader(VertexShader newVertexShader)
{
    m_vertexShader = newVertexShader;
}

std::vector<std::array<Vertex, 3>> VertexProcessor::ProcessTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, const glm::mat4& mvp)
{
    Vertex tv1 = m_vertexShader ? m_vertexShader(v1, mvp) : v1;
    Vertex tv2 = m_vertexShader ? m_vertexShader(v2, mvp) : v2;
    Vertex tv3 = m_vertexShader ? m_vertexShader(v3, mvp) : v3;

    std::vector<std::array<Vertex, 3>> resultingTriangles;

    Vertex poly1[10];
    Vertex poly2[10];

    poly1[0] = tv1;
    poly1[1] = tv2;
    poly1[2] = tv3;

    int vertexCount = 3;

    for (int i = 0; i < 6; i++) {
        vertexCount = ClipAgainstPlane(m_planes[i], poly1, poly2, vertexCount);
        if (vertexCount < 3)
            return resultingTriangles; // Le triangle est entièrement en dehors du frustum

        for (int j = 0; j < vertexCount; j++) {
            poly1[j] = poly2[j];
        }
    }

    // 3. Décomposition du polygone clippé en triangles (Fan triangulation) et Backface Culling
    for (int i = 1; i < vertexCount - 1; i++) {
        Vertex vertexA = poly1[0];
        Vertex vertexB = poly1[i];
        Vertex vertexC = poly1[i + 1];

        if (!IsBackFace(vertexA, vertexB, vertexC)) {
            resultingTriangles.push_back(std::array<Vertex, 3>{ vertexA, vertexB, vertexC });
        }
    }

    return resultingTriangles;
}

bool VertexProcessor::IsTriangleVisible(const Vertex& v1, const Vertex& v2, const Vertex& v3, const glm::mat4& mvp)
{
    // 1. Appliquer le Vertex Shader sur les sommets
    Vertex tv1 = m_vertexShader ? m_vertexShader(v1, mvp) : v1;
    Vertex tv2 = m_vertexShader ? m_vertexShader(v2, mvp) : v2;
    Vertex tv3 = m_vertexShader ? m_vertexShader(v3, mvp) : v3;

    // 2. Tester contre les 6 plans de clipping (Frustum Culling)
    for (int i = 0; i < 6; i++) {
        ClipPlane plane = m_planes[i];
        float d1 = PlaneDist(tv1, plane);
        float d2 = PlaneDist(tv2, plane);
        float d3 = PlaneDist(tv3, plane);

        // Si les 3 sommets sont strictement en dehors du même plan, le triangle est invisible
        if (d1 < 0.0f && d2 < 0.0f && d3 < 0.0f) {
            return false;
        }
    }

    return true; // Le triangle est partiellement ou totalement dans le champ de vision
}

Vertex VertexProcessor::TransformVertex(
    const Vertex& vertex,
    const glm::mat4& mvp) const
{
    return m_vertexShader
        ? m_vertexShader(vertex, mvp)
        : vertex;
}

bool VertexProcessor::IsTriangleVisibleClipSpace(
    const Vertex& tv1,
    const Vertex& tv2,
    const Vertex& tv3) const
{
    for (int i = 0; i < 6; i++)
    {
        ClipPlane plane = m_planes[i];

        float d1 = PlaneDist(tv1, plane);
        float d2 = PlaneDist(tv2, plane);
        float d3 = PlaneDist(tv3, plane);

        // Les trois sommets sont hors du même plan.
        if (d1 < 0.0f && d2 < 0.0f && d3 < 0.0f)
            return false;
    }

    return true;
}

std::vector<std::array<Vertex, 3>>
VertexProcessor::ProcessTransformedTriangle(
    const Vertex& tv1,
    const Vertex& tv2,
    const Vertex& tv3)
{
    std::vector<std::array<Vertex, 3>> resultingTriangles;

    // 1. Création des buffers temporaires pour le clipping.
    Vertex poly1[10];
    Vertex poly2[10];

    poly1[0] = tv1;
    poly1[1] = tv2;
    poly1[2] = tv3;

    int vertexCount = 3;

    // 2. Clipping contre les 6 plans du Frustum.
    for (int i = 0; i < 6; i++)
    {
        vertexCount = ClipAgainstPlane(
            m_planes[i],
            poly1,
            poly2,
            vertexCount
        );

        // Le triangle est entièrement hors du Frustum.
        if (vertexCount < 3)
            return resultingTriangles;

        // On récupère le polygone résultant.
        for (int j = 0; j < vertexCount; j++)
        {
            poly1[j] = poly2[j];
        }
    }

    // 3. Triangulation du polygone (Triangle Fan)
    // et Backface Culling.
    for (int i = 1; i < vertexCount - 1; i++)
    {
        Vertex vertexA = poly1[0];
        Vertex vertexB = poly1[i];
        Vertex vertexC = poly1[i + 1];

        if (!IsBackFace(vertexA, vertexB, vertexC))
        {
            resultingTriangles.push_back({
                vertexA,
                vertexB,
                vertexC
                });
        }
    }

    return resultingTriangles;
}

float VertexProcessor::PlaneDist(const Vertex& vertex, ClipPlane plane) const
{
    switch (plane)
    {
    case ClipPlane::Near:   return vertex.position.w + vertex.position.z;
    case ClipPlane::Far:    return vertex.position.w - vertex.position.z;
    case ClipPlane::Left:   return vertex.position.w + vertex.position.x;
    case ClipPlane::Right:  return vertex.position.w - vertex.position.x;
    case ClipPlane::Bottom: return vertex.position.w + vertex.position.y;
    case ClipPlane::Top:    return vertex.position.w - vertex.position.y;
    }
    return 0.0f;
}

Vertex VertexProcessor::CrossPlane(const Vertex& vertexFront, const Vertex& vertexBack, ClipPlane plane)
{
    float distanceFront = PlaneDist(vertexFront, plane);
    float distanceBack = PlaneDist(vertexBack, plane);
    return Mix(vertexFront, vertexBack, distanceFront / (distanceFront - distanceBack));
}

int VertexProcessor::ClipAgainstPlane(ClipPlane plane, const Vertex* inVertices, Vertex* outVertices, int inCount)
{
    if (inCount == 0) return 0;

    int newVertices = 0;
    for (int i = 0; i < inCount; i++) {
        const Vertex& startPoint = inVertices[i];
        const Vertex& endPoint = inVertices[(i + 1) % inCount];

        bool startPointIn = PlaneDist(startPoint, plane) > 0;
        bool endPointIn = PlaneDist(endPoint, plane) > 0;

        if (startPointIn)
        {
            outVertices[newVertices++] = startPoint;
        }

        if (startPointIn != endPointIn)
        {
            outVertices[newVertices++] = CrossPlane(startPoint, endPoint, plane);
        }
    }
    return newVertices;
}

bool VertexProcessor::IsBackFace(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3)
{
    float vertex1X = vertex1.position.x / vertex1.position.w;
    float vertex1Y = vertex1.position.y / vertex1.position.w;

    float vertex2X = vertex2.position.x / vertex2.position.w;
    float vertex2Y = vertex2.position.y / vertex2.position.w;

    float vertex3X = vertex3.position.x / vertex3.position.w;
    float vertex3Y = vertex3.position.y / vertex3.position.w;

    float area2 = (vertex2X - vertex1X) * (vertex3Y - vertex1Y) - (vertex2Y - vertex1Y) * (vertex3X - vertex1X);

    return area2 <= 0.0f;
}