#include "stdafx.h"
#include "ControllerState.h"
#include "Application.h"

void ControllerState::update(vr::TrackedDeviceIndex_t deviceIndex, const glm::mat4 & pose)
{
	m_lastState = m_currentState;
	if (!g_pHMD->GetControllerState(deviceIndex, &m_currentState, sizeof(m_currentState)))
	{
		clearState();
	}

	translateTrackpadToDPad();

	m_controllerToRoom = pose;
}

bool ControllerState::isButtonDown(vr::EVRButtonId button) const
{
	return (m_currentState.ulButtonPressed & vr::ButtonMaskFromId(button)) != 0;
}

bool ControllerState::wasButtonPressed(vr::EVRButtonId button) const
{
	return ((m_currentState.ulButtonPressed & ~m_lastState.ulButtonPressed) & vr::ButtonMaskFromId(button)) != 0;
}

bool ControllerState::wasButtonReleased(vr::EVRButtonId button) const
{
	return ((~m_currentState.ulButtonPressed & m_lastState.ulButtonPressed) & vr::ButtonMaskFromId(button)) != 0;
}

vr::VRControllerAxis_t ControllerState::getTrackpad() const
{
	return m_currentState.rAxis[vr::k_EButton_SteamVR_Touchpad - vr::k_EButton_Axis0];
}

float ControllerState::getTrigger() const
{
	return m_currentState.rAxis[vr::k_EButton_SteamVR_Trigger - vr::k_EButton_Axis0].x;
}

void ControllerState::clearState()
{
	memset(&m_currentState, 0, sizeof(m_currentState));
}

void ControllerState::translateTrackpadToDPad()
{
	if (isButtonDown(vr::k_EButton_SteamVR_Touchpad))
	{
		vr::EVRButtonId padButton;
		const auto& axis = getTrackpad();
		if (abs(axis.x) > abs(axis.y))
		{
			padButton = (axis.x > 0) ? vr::k_EButton_DPad_Right : vr::k_EButton_DPad_Left;
		}
		else
		{
			padButton = (axis.y > 0) ? vr::k_EButton_DPad_Up : vr::k_EButton_DPad_Down;
		}

		m_currentState.ulButtonPressed |= vr::ButtonMaskFromId(padButton);
	}
}

