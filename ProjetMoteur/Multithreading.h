#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class Multithreading {
public:
    static Multithreading& GetInstance();

    void EnqueueTask(std::function<void()> task);

    void WaitAll();

    void DispatchTasks(int height, std::function<void(int taskId, int minY, int maxY)> workerTask);

    int GetNumThreads() const;

private:
    Multithreading();
    ~Multithreading();

    void WorkerThreadLoop();

    static constexpr int m_NUM_TASKS = 16;

    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_taskQueue;

    std::mutex m_queueMutex;                         // Verrou pour protéger l'accès à la file
    std::condition_variable m_condition;             // Pour réveiller les threads endormis
    bool m_stopThreads = false;                      // Signal pour tuer les threads à la fermeture

    std::atomic<int> m_tasksCompleted{ 0 };            // Compteur des tâches terminées
    std::mutex m_waitMutex;                          // Verrou pour le thread principal
    std::condition_variable m_waitCondition;         // Pour réveiller le main() quand la frame est finie

    Multithreading(const Multithreading&) = delete;
    Multithreading& operator=(const Multithreading&) = delete;
};