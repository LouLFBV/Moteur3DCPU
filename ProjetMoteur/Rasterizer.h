#pragma once
#include <vector>
#include <array>
#include <cstdint>
#include "Types.h"
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

struct RenderStats {
    int trianglesProcessed = 0;
    int trianglesRasterized = 0;
    std::atomic<int> pixelsProcessed{ 0 }; // Protégé contre les accès simultanés 
    int threadsUsed = 1;
    float frameTimeMs = 0.0f;
};

enum class ClipPlane {
    Near,
    Far,
    Left,
    Right,
    Top,
    Bottom 
};

class Rasterizer
{
public:
    Rasterizer(int width, int height);
    ~Rasterizer();

    void ResetStats();
    const RenderStats& GetStats() const;

    void Clear();
    void ClearZBuffer();
    void DrawTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3);
    void DrawSquare(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, const Vertex& vertex4);
    void DrawCube(const Cube& cube);

    const void* GetPixels() const;
    int GetPitch() const;

    void RenderScene();

private:
    ScreenVertex ToScreen(const Vertex& vertex);
    void Plot(int x, int y, float z, float r, float g, float b);
    void SortByY(ScreenVertex& sv1, ScreenVertex& sv2, ScreenVertex& sv3);
    bool MiddleIsRight(const ScreenVertex& sv1, const ScreenVertex& sv2, const ScreenVertex& sv3);
    void WalkEdge(const ScreenVertex& sv1, const ScreenVertex& sv2, BordLigne* out) const;
    void FillRows(int y0, int y1, const BordLigne* left, const BordLigne* right, int tileMinY, int tileMaxY);
    void FillRowFast(int y, const BordLigne& l, const BordLigne& r);

    Vertex Mix(const Vertex& vertex1, const Vertex& vertex2, float t);
    float PlaneDist(const Vertex& vertex, ClipPlane plane); 
    Vertex CrossPlane(const Vertex& p, const Vertex& q, ClipPlane plane);
    int ClipAgainstPlane(ClipPlane plane, const Vertex* inVertices, Vertex* outVertices, int inCount);

    void RasterizeTriangle(const Vertex& vertex1, const Vertex& vertex2, const Vertex& vertex3, int tileMinY, int tileMaxY, std::vector<BordLigne>& longSide, std::vector<BordLigne>& shortSide);

    void WorkerThreadLoop();


    // On peut maintenant avoir plus de tâches que de threads ! (ex: 16 bandes pour 8 threads)
    static constexpr int m_NUM_TASKS = 16;

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

    std::vector<std::thread> m_workers;              
    std::queue<std::function<void()>> m_taskQueue;  

    std::mutex m_queueMutex;                         // Verrou pour protéger l'accès à la file
    std::condition_variable m_condition;             // Pour réveiller les threads endormis
    bool m_stopThreads = false;                      // Signal pour tuer les threads à la fermeture

    std::atomic<int> m_tasksCompleted{ 0 };            // Compteur des tâches terminées
    std::mutex m_waitMutex;                          // Verrou pour le thread principal
    std::condition_variable m_waitCondition;         // Pour réveiller le main() quand la frame est finie

};