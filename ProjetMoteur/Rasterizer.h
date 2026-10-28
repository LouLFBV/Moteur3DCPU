#pragma once
#include <vector>
#include <array>
#include <cstdint>
#include "Types.h"
#include "Multithreading.h"
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class Rasterizer
{
public:
    using PixelShader = std::function<uint32_t(const BordLigne&)>;

    Rasterizer(int width, int height);
    ~Rasterizer();

    void ResetStats();
    const RenderStats& GetStats() const;

    void Clear();
    void ClearZBuffer();
    void DrawTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3);

    const void* GetPixels() const;
    int GetPitch() const;
    void SetPixelShader(PixelShader shader);

    void RenderScene();

private:
    ScreenVertex ToScreen(const Vertex& vertex);
    void Plot(int x, int y, float z, uint32_t color);
    void SortByY(ScreenVertex& sv1, ScreenVertex& sv2, ScreenVertex& sv3);
    bool MiddleIsRight(const ScreenVertex& sv1, const ScreenVertex& sv2, const ScreenVertex& sv3);
    void WalkEdge(const ScreenVertex& sv1, const ScreenVertex& sv2, BordLigne* out) const;
    void FillRows(int y0, int y1, const BordLigne* left, const BordLigne* right, int tileMinY, int tileMaxY);
    void FillRowFast(int y, const BordLigne& l, const BordLigne& r);
    bool IsBackFace(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3);

    Vertex Mix(const Vertex& vertex1, const Vertex& vertex2, float t);
    float PlaneDist(const Vertex& vertex, ClipPlane plane); 
    Vertex CrossPlane(const Vertex& p, const Vertex& q, ClipPlane plane);
    int ClipAgainstPlane(ClipPlane plane, const Vertex* inVertices, Vertex* outVertices, int inCount);

    void RasterizeTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, int tileMinY, int tileMaxY, std::vector<BordLigne>& longSide, std::vector<BordLigne>& shortSide);

    void WorkerThreadLoop();


    

    int m_width;
    int m_height;
    std::vector<uint32_t> m_framebuffer;
    std::vector<float> m_zbuffer;
    std::vector<std::array<Vertex, 3>> m_triangleQueue;
    static constexpr ClipPlane m_planes[6] = {
        ClipPlane::Near,
        ClipPlane::Far,
        ClipPlane::Left,
        ClipPlane::Right,
        ClipPlane::Top,
        ClipPlane::Bottom
    };
    RenderStats m_stats;    

    PixelShader m_currentShader;
};