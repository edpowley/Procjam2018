#pragma once

class ControllerState
{
public:
	void update(vr::TrackedDeviceIndex_t deviceIndex, const glm::mat4& pose);

	bool isButtonDown(vr::EVRButtonId button) const;
	bool wasButtonPressed(vr::EVRButtonId button) const;
	bool wasButtonReleased(vr::EVRButtonId button) const;

	vr::VRControllerAxis_t getTrackpad() const;
	float getTrigger() const;

	glm::mat4 getControllerToRoom() const { return m_controllerToRoom; }

private:
	vr::VRControllerState_t m_currentState, m_lastState;

	glm::mat4 m_controllerToRoom;

	void clearState();
	void translateTrackpadToDPad();
};
