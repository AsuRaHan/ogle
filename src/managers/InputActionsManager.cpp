#include "managers/InputActionsManager.h"

#include "managers/CameraManager.h"
#include "input/InputAction.h"
#include "input/InputController.h"

#include <windows.h>

void InputActionsManager::ConfigureDefaultActions()
{
    if (m_isConfigured) {
        return;
    }

    auto& input = OGLE::InputController::Get();
    OGLE::InputAction* closeAction = input.GetAction("CloseApplication");
    if (!closeAction) {
        closeAction = input.CreateAction("CloseApplication", OGLE::ActionType::Trigger);
    }

    if (closeAction) {
        closeAction->AddKey(VK_ESCAPE);
        closeAction->OnPressed([](const OGLE::ActionState&) {
            PostQuitMessage(0);
        });
    }

    OGLE::InputAction* moveForwardAction = input.GetAction("CameraMoveForward");
    if (!moveForwardAction) {
        moveForwardAction = input.CreateAction("CameraMoveForward", OGLE::ActionType::Axis);
    }
    if (moveForwardAction) {
        moveForwardAction->AddAxisPair('W', 'S');
    }

    OGLE::InputAction* moveRightAction = input.GetAction("CameraMoveRight");
    if (!moveRightAction) {
        moveRightAction = input.CreateAction("CameraMoveRight", OGLE::ActionType::Axis);
    }
    if (moveRightAction) {
        moveRightAction->AddAxisPair('D', 'A');
    }

    OGLE::InputAction* moveUpAction = input.GetAction("CameraMoveUp");
    if (!moveUpAction) {
        moveUpAction = input.CreateAction("CameraMoveUp", OGLE::ActionType::Axis);
    }
    if (moveUpAction) {
        moveUpAction->AddAxisPair('E', 'Q');
    }

    OGLE::InputAction* lookHoldAction = input.GetAction("CameraLookHold");
    if (!lookHoldAction) {
        lookHoldAction = input.CreateAction("CameraLookHold", OGLE::ActionType::Button);
    }
    if (lookHoldAction) {
        lookHoldAction->AddMouseButton(OGLE::MouseButton::Right);
    }

    OGLE::InputAction* moveBoostAction = input.GetAction("CameraMoveBoost");
    if (!moveBoostAction) {
        moveBoostAction = input.CreateAction("CameraMoveBoost", OGLE::ActionType::Button);
    }
    if (moveBoostAction) {
        moveBoostAction->AddKey(VK_SHIFT);
    }

    m_isConfigured = true;
}

void InputActionsManager::UpdateCameraControls(CameraManager& cameraManager, float deltaTime)
{
    auto& input = OGLE::InputController::Get();

    cameraManager.SetMode(OGLE::Camera::Mode::Free);
    cameraManager.GetCamera().SetMouseSensitivity(0.08f);

    float movementSpeed = cameraManager.GetCamera().GetMovementSpeed();
    if (const OGLE::InputAction* boostAction = input.GetAction("CameraMoveBoost")) {
        if (boostAction->GetState().active) {
            movementSpeed *= 3.0f;
        }
    }

    if (const OGLE::InputAction* moveForwardAction = input.GetAction("CameraMoveForward")) {
        const float value = moveForwardAction->GetState().value;
        if (value != 0.0f) {
            cameraManager.MoveForward(value * movementSpeed * deltaTime);
        }
    }

    if (const OGLE::InputAction* moveRightAction = input.GetAction("CameraMoveRight")) {
        const float value = moveRightAction->GetState().value;
        if (value != 0.0f) {
            cameraManager.MoveRight(value * movementSpeed * deltaTime);
        }
    }

    if (const OGLE::InputAction* moveUpAction = input.GetAction("CameraMoveUp")) {
        const float value = moveUpAction->GetState().value;
        if (value != 0.0f) {
            cameraManager.MoveUp(value * movementSpeed * deltaTime);
        }
    }

    const bool canLook = [&input]() {
        const OGLE::InputAction* lookHoldAction = input.GetAction("CameraLookHold");
        return lookHoldAction && lookHoldAction->GetState().active;
    }();

    if (canLook) {
        const glm::vec2 mouseDelta = input.GetMouseDelta();
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
            cameraManager.GetCamera().ProcessMouseMovement(mouseDelta.x, -mouseDelta.y);
        }
    }

    const float wheelDelta = input.GetMouseWheelDelta();
    if (wheelDelta != 0.0f) {
        cameraManager.GetCamera().ProcessMouseScroll(wheelDelta);
    }
}
