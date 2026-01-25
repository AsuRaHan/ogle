// src/core/EventSystem.h
#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>
#include "log/Logger.h"

namespace ogle {

class EventSystem {
public:
    using EventID = size_t;
    using Callback = std::function<void()>;
    
    struct EventListener {
        EventID id;
        Callback callback;
        int priority = 0; // Для порядка вызова
    };
    
    // Подписка с приоритетом
    EventID Subscribe(const std::string& eventName, Callback callback, int priority = 0) {
        static EventID nextID = 1;
        EventID id = nextID++;
        
        m_listeners[eventName].push_back({id, callback, priority});
        
        // Сортируем по приоритету (высший приоритет = раньше вызывается)
        auto& listeners = m_listeners[eventName];
        std::sort(listeners.begin(), listeners.end(),
            [](const EventListener& a, const EventListener& b) {
                return a.priority > b.priority;
            });
        
        Logger::Debug("Event subscribed: " + eventName + " (ID: " + 
                     std::to_string(id) + ")");
        
        return id;
    }
    
    // Отписка
    void Unsubscribe(const std::string& eventName, EventID id) {
        auto it = m_listeners.find(eventName);
        if (it != m_listeners.end()) {
            auto& listeners = it->second;
            listeners.erase(
                std::remove_if(listeners.begin(), listeners.end(),
                    [id](const EventListener& listener) {
                        return listener.id == id;
                    }),
                listeners.end());
            
            Logger::Debug("Event unsubscribed: " + eventName + " (ID: " + 
                         std::to_string(id) + ")");
        }
    }
    
    // Вызов события
    void Emit(const std::string& eventName) {
        auto it = m_listeners.find(eventName);
        if (it != m_listeners.end()) {
            for (auto& listener : it->second) {
                try {
                    listener.callback();
                } catch (const std::exception& e) {
                    Logger::Error("Event callback failed: " + 
                                 std::string(e.what()));
                }
            }
            
            Logger::Debug("Event emitted: " + eventName + 
                         " (" + std::to_string(it->second.size()) + " listeners)");
        }
    }
    
    // Проверка наличия слушателей
    bool HasListeners(const std::string& eventName) const {
        auto it = m_listeners.find(eventName);
        return it != m_listeners.end() && !it->second.empty();
    }
    
    // Очистка всех слушателей события
    void ClearEvent(const std::string& eventName) {
        m_listeners.erase(eventName);
        Logger::Debug("Event cleared: " + eventName);
    }
    
    // Очистка всех событий
    void ClearAll() {
        m_listeners.clear();
        Logger::Info("All events cleared");
    }
    
private:
    std::unordered_map<std::string, std::vector<EventListener>> m_listeners;
};

} // namespace ogle