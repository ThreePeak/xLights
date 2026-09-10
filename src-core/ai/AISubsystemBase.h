#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "aiBase.h"
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <mutex>
#include <atomic>

class ServiceManager;

/**
 * @brief Abstract Base Class for all AI Subsystems in xLights.
 * Provides async initialization, capability query contracts, thread safety,
 * and service lifecycle management for pure C++ core engines.
 */
class AISubsystemBase {
public:
    using StatusCallback = std::function<void(const std::string& message, float progressPercentage)>;

    explicit AISubsystemBase(ServiceManager* serviceManager = nullptr)
        : m_serviceManager(serviceManager), m_isInitialized(false) {}
    
    virtual ~AISubsystemBase() = default;

    AISubsystemBase(const AISubsystemBase&) = delete;
    AISubsystemBase& operator=(const AISubsystemBase&) = delete;
    AISubsystemBase(AISubsystemBase&&) noexcept = default;
    AISubsystemBase& operator=(AISubsystemBase&&) noexcept = default;

    /**
     * @brief Asynchronously initializes the AI subsystem and loads models/weights.
     */
    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) = 0;

    /**
     * @brief Shuts down the AI subsystem and releases associated memory/GPU resources.
     */
    virtual void Shutdown() = 0;

    /**
     * @brief Returns true if the subsystem is ready for inference or processing tasks.
     */
    [[nodiscard]] virtual bool IsReady() const {
        return m_isInitialized.load();
    }

    /**
     * @brief Requests cooperative cancellation of any ongoing background work.
     */
    virtual void RequestCancel() {
        m_cancelRequested.store(true);
    }

    /**
     * @brief Checks if cancellation has been requested for this subsystem.
     */
    [[nodiscard]] virtual bool IsCancelled() const {
        return m_cancelRequested.load();
    }

    /**
     * @brief Resets the cancellation flag before starting a new task.
     */
    virtual void ResetCancel() {
        m_cancelRequested.store(false);
    }

    /**
     * @brief Returns the human-readable identifier of the AI subsystem.
     */
    [[nodiscard]] virtual std::string GetSubsystemName() const = 0;

    /**
     * @brief Returns a list of capability tokens supported by this subsystem.
     */
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const = 0;

protected:
    ServiceManager* m_serviceManager{nullptr};
    std::atomic<bool> m_isInitialized{false};
    std::atomic<bool> m_cancelRequested{false};
    mutable std::mutex m_subsystemMutex;
};
