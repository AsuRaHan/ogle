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

    auto& input = ogle::InputController::Get();
    ogle::InputAction* closeAction = input.GetAction("CloseApplication");
    if (!closeAction) {
        closeAction = input.CreateAction("CloseApplication", ogle::ActionType::Trigger);
    }

    if (closeAction) {
        closeAction->AddKey(VK_ESCAPE);
        closeAction->OnPressed([](const ogle::ActionState&) {
            PostQuitMessage(0);
        });
    }

    ogle::InputAction* moveForwardAction = input.GetAction("CameraMoveForward");
    if (!moveForwardAction) {
        moveForwardAction = input.CreateAction("CameraMoveForward", ogle::ActionType::Axis);
    }
    if (moveForwardAction) {
        moveForwardAction->AddAxisPair('W', 'S');
    }

    ogle::InputAction* moveRightAction = input.GetAction("CameraMoveRight");
    if (!moveRightAction) {
        moveRightAction = input.CreateAction("CameraMoveRight", ogle::ActionType::Axis);
    }
    if (moveRightAction) {
        moveRightAction->AddAxisPair('D', 'A');
    }

    ogle::InputAction* moveUpAction = input.GetAction("CameraMoveUp");
    if (!moveUpAction) {
        moveUpAction = input.CreateAction("CameraMoveUp", ogle::ActionType::Axis);
    }
    if (moveUpAction) {
        moveUpAction->AddAxisPair('E', 'Q');
    }

    ogle::InputAction* lookHoldAction = input.GetAction("CameraLookHold");
    if (!lookHoldAction) {
        lookHoldAction = input.CreateAction("CameraLookHold", ogle::ActionType::Button);
    }
    if (lookHoldAction) {
        lookHoldAction->AddMouseButton(ogle::MouseButton::Right);
    }

    ogle::InputAction* moveBoostAction = input.GetAction("CameraMoveBoost");
    if (!moveBoostAction) {
        moveBoostAction = input.CreateAction("CameraMoveBoost", ogle::ActionType::Button);
    }
    if (moveBoostAction) {
        moveBoostAction->AddKey(VK_SHIFT);
    }

    m_isConfigured = true;
}

void InputActionsManager::UpdateCameraControls(CameraManager& cameraManager, float deltaTime)
{
    auto& input = ogle::InputController::Get();

    cameraManager.SetMode(ogle::Camera::Mode::Free);
    cameraManager.GetCamera().SetMouseSensitivity(0.08f);

    float movementSpeed = cameraManager.GetCamera().GetMovementSpeed();
    if (const ogle::InputAction* boostAction = input.GetAction("CameraMoveBoost")) {
        if (boostAction->GetState().active) {
            movementSpeed *= 3.0f;
        }
    }

    if (const ogle::InputAction* moveForwardAction = input.GetAction("CameraMoveForward")) {
        const float value = moveForwardAction->GetState().value;
        if (value != 0.0f) {
            cameraManager.MoveForward(value * movementSpeed * deltaTime);
        }
    }

    if (const ogle::InputAction* moveRightAction = input.GetAction("CameraMoveRight")) {
        const float value = moveRightAction->GetState().value;
        if (value != 0.0f) {
            cameraManager.MoveRight(value * movementSpeed * deltaTime);
        }
    }

    if (const ogle::InputAction* moveUpAction = input.GetAction("CameraMoveUp")) {
        const float value = moveUpAction->GetState().value;
        if (value != 0.0f) {
            cameraManager.MoveUp(value * movementSpeed * deltaTime);
        }
    }

    const bool canLook = [&input]() {
        const ogle::InputAction* lookHoldAction = input.GetAction("CameraLookHold");
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
