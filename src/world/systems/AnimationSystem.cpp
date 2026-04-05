#include "AnimationSystem.h"
#include <entt/entt.hpp>
#include "world/WorldComponents.h"
#include <cmath>

namespace OGLE {
    AnimationSystem::AnimationSystem(entt::basic_registry<>& registry) : m_registry(registry) {}

    void AnimationSystem::Update(float deltaTime) {
        auto animationView = m_registry.view<AnimationComponent, WorldObjectComponent>();
        for (auto entity : animationView) {
            const auto& object = animationView.get<WorldObjectComponent>(entity);
            if (!object.enabled) {
                continue;
            }
            auto& animation = animationView.get<AnimationComponent>(entity);
            if (!animation.enabled || !animation.playing) {
                continue;
            }

            animation.currentTime += animation.playbackSpeed * deltaTime;

            float maxTime = animation.duration > 0.0f ? animation.duration : 3600.0f;
            if (animation.currentClipIndex >= 0 && static_cast<size_t>(animation.currentClipIndex) < animation.clips.size()) {
                maxTime = animation.clips[animation.currentClipIndex].duration;
            }

            if (animation.loop) {
                if (animation.currentTime >= maxTime) {
                    if (maxTime > 0.0f) {
                        animation.currentTime = fmod(animation.currentTime, maxTime);
                    } else {
                        animation.currentTime = 0.0f;
                    }
                }
            } else {
                if (animation.currentTime >= maxTime) {
                    animation.currentTime = maxTime;
                    animation.playing = false;
                }
            }

            if (animation.currentTime < 0.0f) {
                animation.currentTime = 0.0f;
            }
        }
    }
}