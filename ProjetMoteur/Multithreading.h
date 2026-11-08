#pragma once
#include <thread>
#include <vector>
#include <deque>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <exception>

using Job = std::function<void()>;

class Multithreading
{
public:
    static Multithreading& GetInstance();

    void Submit(Job job, bool urgent = false);

    void WaitAll();

    void DispatchTasks(int numTasks, int height, std::function<void(int taskId, int minY, int maxY)> workerTask);

    int GetNumThreads() const;
    int GetNumJobs() const;

private:
    Multithreading();
    ~Multithreading();

    void WorkerThreadLoop();

    int m_numJobs = 0;

    std::vector<std::thread> m_workers;
    std::deque<Job> m_taskQueue;

    std::mutex m_queueMutex;
    std::condition_variable m_condition;

    bool m_stopThreads = false;

    std::atomic<int> m_pendingTasks{ 0 };

    std::mutex m_waitMutex;
    std::condition_variable m_waitCondition;

    std::exception_ptr m_taskException;
    std::mutex m_exceptionMutex;

    Multithreading(const Multithreading&) = delete;
    Multithreading& operator=(const Multithreading&) = delete;
};
