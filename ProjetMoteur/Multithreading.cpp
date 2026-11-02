#include "Multithreading.h"

Multithreading& Multithreading::GetInstance()
{
    static Multithreading instance;
    return instance;
}

Multithreading::Multithreading()
{
    unsigned int cores = std::thread::hardware_concurrency();
    int numThreads = (cores == 0) ? 4 : static_cast<int>(cores);

    for (int i = 0; i < numThreads; ++i) {
        m_workers.emplace_back([this] { WorkerThreadLoop(); });
    }
    M_NUM_JOBS = numThreads * 2;
}

Multithreading::~Multithreading()
{
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_stopThreads = true;
    }
    m_condition.notify_all();

    for (std::thread& worker : m_workers) {
        worker.join();
    }
}

void Multithreading::Submit(Job job, bool urgent)
{
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (urgent) {
            m_taskQueue.push_front(std::move(job)); // Passe devant
        }
        else {
            m_taskQueue.push_back(std::move(job));  // A la suite
        }
        m_pendingTasks++; // Un job de plus en vol
    }
    m_condition.notify_one(); // Réveille un worker
}

void Multithreading::WaitAll()
{
    std::unique_lock<std::mutex> lock(m_waitMutex);
    m_waitCondition.wait(lock, [this] {
        return m_pendingTasks.load() == 0; // On attend que la file soit totalement vide
        });
}

void Multithreading::DispatchTasks(
    int numTasks,
    int height,
    std::function<void(int taskId, int minY, int maxY)> workerTask)
{
    if (height <= 0 || numTasks <= 0 || !workerTask)
        return;

    // On ne crée jamais plus de tâches que de lignes disponibles.
    numTasks = std::min(numTasks, height);

    for (int i = 0; i < numTasks; ++i) {

        // Répartition équilibrée de toutes les lignes.
        int minY = static_cast<int>(
            (static_cast<long long>(i) * height) / numTasks
            );

        int maxY = static_cast<int>(
            (static_cast<long long>(i + 1) * height) / numTasks
            );

        Submit([i, minY, maxY, workerTask]() {
            workerTask(i, minY, maxY);
            });
    }
}

int Multithreading::GetNumThreads() const
{
    return static_cast<int>(m_workers.size());
}

int Multithreading::GetNumJobs() const
{
    return M_NUM_JOBS;
}

void Multithreading::WorkerThreadLoop()
{
    for (;;) {
        Job task;

        // 1. On cherche une tâche
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_condition.wait(lock, [this] {
                return m_stopThreads || !m_taskQueue.empty();
                });

            if (m_stopThreads && m_taskQueue.empty())
                return;

            task = std::move(m_taskQueue.front());
            m_taskQueue.pop_front();
        }

        // 2. On exécute
        task();

        // 3. On décrémente et on signale si on a tout fini
        {
            std::lock_guard<std::mutex> lock(m_waitMutex);
            m_pendingTasks--;
        }
        m_waitCondition.notify_all(); // notify_all est plus sûr ici pour réveiller WaitAll()
    }
}