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
}

Multithreading::~Multithreading()
{
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_stopThreads = true; // On donne l'ordre d'arrêt
    }
    m_condition.notify_all(); // On réveille tout le monde pour qu'ils voient l'ordre d'arrêt

    for (std::thread& worker : m_workers) {
        worker.join(); // On attend qu'ils aient fini de ranger leurs affaires
    }
}

void Multithreading::EnqueueTask(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_taskQueue.push(std::move(task));
    }
    m_condition.notify_one();
}

void Multithreading::WaitAll()
{
    std::unique_lock<std::mutex> lock(m_waitMutex);
    m_waitCondition.wait(lock, [this] {
        return m_tasksCompleted.load() == m_NUM_TASKS;
        });
}

void Multithreading::DispatchTasks(int height, std::function<void(int taskId, int minY, int maxY)> workerTask)
{
    m_tasksCompleted.store(0);
    int bandHeight = height / m_NUM_TASKS;

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        for (int i = 0; i < m_NUM_TASKS; ++i) {
            int minY = i * bandHeight;
            int maxY = (i == m_NUM_TASKS - 1) ? height : (i + 1) * bandHeight;

            m_taskQueue.push([i, minY, maxY, workerTask]() {
                workerTask(i, minY, maxY);
                });
        }
    }
    m_condition.notify_all();
}

int Multithreading::GetNumThreads() const
{
    return static_cast<int>(m_workers.size());
}


void Multithreading::WorkerThreadLoop()
{
    for (;;) {
        std::function<void()> task;

        // 1. On cherche une tâche dans la file
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);

            // Le thread s'endort ici tant que la file est vide (et qu'on ne l'arrête pas)
            m_condition.wait(lock, [this]
                {
                    return m_stopThreads || !m_taskQueue.empty();
                });

            // Si on a reçu l'ordre de fermer et qu'il n'y a plus de travail, on quitte la boucle
            if (m_stopThreads && m_taskQueue.empty())
                return;

            // On récupère la tâche tout en haut de la pile
            task = std::move(m_taskQueue.front());
            m_taskQueue.pop();
        }

        // 2. On exécute la tâche (sans verrouiller le reste du programme !)
        task();

        // 3. On signale au thread principal (le main) qu'on a terminé cette tâche
        {
            std::lock_guard<std::mutex> lock(m_waitMutex);
            m_tasksCompleted++;
        }
        //Prend un thread endormi au hazard et il prend le mutex et fais son teste, si c'est vrai, il continue, sinon il dort
        m_waitCondition.notify_one();
    }
}
