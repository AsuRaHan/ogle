#include "scripting/bindings/AnimationBindings.h"
#include "world/IWorldAccess.h"
#include "world/World.h"
#include "world/WorldComponents.h"
#include "render/AnimationLibrary.h"

namespace OGLE
{
    namespace ScriptBindings
    {
        void RegisterAnimationBindings(duktape::api_object& ns, IWorldAccess& worldAccess, PhysicsManager& physics)
        {
            ns.add_function("createAnimationAsset", [&](const std::string& name, Entity entity) {
                auto& world = worldAccess.GetActiveWorld();
                if (AnimationComponent* animComp = world.GetAnimation(entity)) {
                    return AnimationLibrary::Instance().AddAnimation(name, *animComp);
                }
                return false;
            });

            ns.add_function("applyAnimationAsset", [&](const std::string& name, Entity entity) {
                auto& world = worldAccess.GetActiveWorld();
                AnimationComponent* asset = AnimationLibrary::Instance().GetAnimation(name);
                if (!asset) {
                    return false;
                }

                if (AnimationComponent* animationComp = world.GetAnimation(entity)) {
                    *animationComp = *asset;
                } else {
                    world.GetRegistry().emplace<AnimationComponent>(entity, *asset);
                }
                return true;
            });

            ns.add_function("saveAnimationLibrary", [](const std::string& path) {
                return AnimationLibrary::Instance().SaveToFile(path);
            });

            ns.add_function("loadAnimationLibrary", [](const std::string& path) {
                return AnimationLibrary::Instance().LoadFromFile(path);
            });
        }
    }
}