#pragma once
#include <thread>
#include <vector>
#include <deque> 
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

using Job = std::function<void()>; // Raccourci plus propre

class Multithreading {
public:
    static Multithreading& GetInstance();

    // Nouveau système de soumission (issu de ton cours)
    void Submit(Job job, bool urgent = false);

    // Attendre que TOUS les jobs en cours soient terminés
    void WaitAll();

    // Helper pour générer N tâches de rastérisation
    void DispatchTasks(int numTasks, int height, std::function<void(int taskId, int minY, int maxY)> workerTask);

    int GetNumThreads() const;

private:
    Multithreading();
    ~Multithreading();

    void WorkerThreadLoop();

    std::vector<std::thread> m_workers;
    std::deque<Job> m_taskQueue; // Deque permet l'insertion en tête ou en queue

    std::mutex m_queueMutex;
    std::condition_variable m_condition;
    bool m_stopThreads = false;

    // Compteur de jobs soumis mais pas encore terminés (remplace m_tasksCompleted et m_NUM_TASKS)
    std::atomic<int> m_pendingTasks{ 0 };
    std::mutex m_waitMutex;
    std::condition_variable m_waitCondition;

    Multithreading(const Multithreading&) = delete;
    Multithreading& operator=(const Multithreading&) = delete;
};