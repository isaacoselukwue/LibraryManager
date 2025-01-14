#ifndef AUDIT_LOGGER_HPP
#define AUDIT_LOGGER_HPP

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "../Interfaces/Audits.hpp"

class AuditLogger {
private:
    Audits& audit;
    bool running;
    std::queue<AuditLogDto> logQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::thread workerThread;

    void ProcessLogs() {
        while (running) {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this]{ return !logQueue.empty() || !running; });

            while (!logQueue.empty()) {
                auto log = logQueue.front();
                logQueue.pop();
                lock.unlock();
                audit.AddAuditLog(log);
                lock.lock();
            }
        }
    }

public:
    AuditLogger(Audits& auditRef) : audit(auditRef), running(true) {
        workerThread = std::thread(&AuditLogger::ProcessLogs, this);
    }

    ~AuditLogger() {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            running = false;
        }
        condition.notify_one();
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

    void LogAsync(AuditLogDto log) {
        std::lock_guard<std::mutex> lock(queueMutex);
        logQueue.push(log);
        condition.notify_one();
    }
};

#endif