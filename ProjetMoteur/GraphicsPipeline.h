#pragma once
#include "Rasterizer.h"
#include <functional>
#include <glm/glm.hpp>
#include <vector>

class GraphicsPipeline
{
public:
    // Signature d'un Vertex Shader : prend un sommet local et la matrice MVP, renvoie le sommet transformé
    using VertexShader = std::function<Vertex(const Vertex& vertex, const glm::mat4& mvp)>;

    // Signature d'un Fragment Shader : prend un pixel interpolé et renvoie une couleur 32-bit
    using FragmentShader = std::function<uint32_t(const BordLigne&)>;

    GraphicsPipeline(int width, int height);

    // Configuration des Shaders à la volée
    void SetVertexShader(VertexShader newVertexShader);
    void SetFragmentShader(FragmentShader newFragmentShader);

    // Méthodes de dessin à travers le Pipeline (Vertex Shader -> Rasterizer)
    void DrawTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const glm::mat4& mvp);
    void DrawSquare(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const Vertex& vertex4, const glm::mat4& mvp);
    void DrawCube(const Cube& cube, const glm::mat4& mvp);

    // Surcharges pour les éléments déjà en NDC (comme le fond ou le sol)
    void DrawSquareNDC(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const Vertex& vertex4);

    // Passerelles vers le rastériseur
    void Clear();
    void ClearZBuffer();
    void RenderScene();
    const void* GetPixels() const;
    int GetPitch() const;
    void ResetStats();
    const RenderStats& GetStats() const;

private:
    Rasterizer m_rasterizer;
    VertexShader m_vertexShader;
};