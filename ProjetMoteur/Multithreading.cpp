#include "Multithreading.h"
#include <algorithm>
#include <utility>

Multithreading& Multithreading::GetInstance()
{
    static Multithreading instance;
    return instance;
}

Multithreading::Multithreading()
{
    const unsigned int cores = std::thread::hardware_concurrency();

    const int numThreads = (cores == 0) ? 4 : static_cast<int>(cores);

    m_numJobs = numThreads * 2;

    for (int i = 0; i < numThreads; ++i) {
        m_workers.emplace_back([this]() {
            WorkerThreadLoop();
            });
    }
}

Multithreading::~Multithreading()
{
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_stopThreads = true;
    }

    m_condition.notify_all();

    for (std::thread& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void Multithreading::Submit(Job job, bool urgent)
{
    if (!job) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);

        if (m_stopThreads) {
            return;
        }
        if (urgent) {
            m_taskQueue.push_front(std::move(job));
        }
        else {
            m_taskQueue.push_back(std::move(job));
        }
        ++m_pendingTasks;
    }
    m_condition.notify_one();
}

void Multithreading::WaitAll()
{
    {
        std::unique_lock<std::mutex> lock(m_waitMutex);

        m_waitCondition.wait(lock, [this]() {
            return m_pendingTasks.load() == 0;
            });
    }

    std::exception_ptr exception;

    {
        std::lock_guard<std::mutex> lock(m_exceptionMutex);

        exception = m_taskException;
        m_taskException = nullptr;
    }

    if (exception) {
        std::rethrow_exception(exception);
    }
}

void Multithreading::DispatchTasks(int numTasks, int height, std::function<void(int taskId, int minY, int maxY)> workerTask)
{
    if (height <= 0 || numTasks <= 0 || !workerTask) {
        return;
    }

    numTasks = std::min(numTasks, height);

    for (int i = 0; i < numTasks; ++i) {
        const int minY = static_cast<int>((static_cast<long long>(i) * height) / numTasks);

        const int maxY = static_cast<int>((static_cast<long long>(i + 1) * height) / numTasks);

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
    return m_numJobs;
}

void Multithreading::WorkerThreadLoop()
{
    while (true) {
        Job task;

        {
            std::unique_lock<std::mutex> lock(m_queueMutex);

            m_condition.wait(lock, [this]() {
                return m_stopThreads || !m_taskQueue.empty();
                });

            if (m_stopThreads && m_taskQueue.empty())
                return;

            task = std::move(m_taskQueue.front());
            m_taskQueue.pop_front();
        }

        try {
            task();
        }
        catch (...) {
            std::lock_guard<std::mutex> lock(m_exceptionMutex);

            if (!m_taskException)
                m_taskException = std::current_exception();
        }

        {
            std::lock_guard<std::mutex> lock(m_waitMutex);
            --m_pendingTasks;
        }

        m_waitCondition.notify_all();
    }
}