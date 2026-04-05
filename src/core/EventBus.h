#pragma once

#include <functional>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <mutex>

// ─────────────────────────────────────────────────────────────────────────────
// EventBus — a simple type-based publish/subscribe dispatcher.
//
// Usage:
//   // Subscribe:
//   bus.Subscribe<MyEvent>([](const MyEvent& e) { ... });
//
//   // Dispatch:
//   bus.Dispatch(MyEvent{ ... });
//
//   // Unsubscribe (optional, returns the same handler token):
//   auto token = bus.Subscribe<MyEvent>(handler);
//   bus.Unsubscribe<MyEvent>(token);
// ─────────────────────────────────────────────────────────────────────────────

class EventBus
{
public:
    using HandlerId = size_t;

    // ── Subscribe ──────────────────────────────────────────────────────────
    // Registers a callback for event type T. Returns a unique token that can
    // be used later to unsubscribe.
    template <typename T>
    HandlerId Subscribe(std::function<void(const T&)> handler)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto& entry = m_subscribers[typeid(T)];
        HandlerId id = m_nextId++;

        entry.handlers.push_back({
            id,
            [handler](const void* evt) { handler(*static_cast<const T*>(evt)); }
        });

        return id;
    }

    // ── Unsubscribe ────────────────────────────────────────────────────────
    // Removes a previously registered handler by its token.
    template <typename T>
    void Unsubscribe(HandlerId id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_subscribers.find(typeid(T));
        if (it == m_subscribers.end())
            return;

        auto& handlers = it->second.handlers;
        handlers.erase(
            std::remove_if(handlers.begin(), handlers.end(),
                [id](const HandlerEntry& e) { return e.id == id; }),
            handlers.end()
        );

        if (handlers.empty())
            m_subscribers.erase(it);
    }

    // ── Dispatch ───────────────────────────────────────────────────────────
    // Immediately invokes all handlers registered for event type T.
    template <typename T>
    void Dispatch(const T& event)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_subscribers.find(typeid(T));
        if (it == m_subscribers.end())
            return;

        for (const auto& entry : it->second.handlers)
            entry.callback(&event);
    }

    // ── Clear ──────────────────────────────────────────────────────────────
    // Removes all subscribers of all event types.
    void Clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_subscribers.clear();
    }

private:
    struct HandlerEntry
    {
        HandlerId id;
        std::function<void(const void*)> callback;
    };

    struct SubscriberList
    {
        std::vector<HandlerEntry> handlers;
    };

    std::unordered_map<std::type_index, SubscriberList> m_subscribers;
    HandlerId m_nextId = 1;
    std::mutex m_mutex;
};
