#pragma once

#include "resource.h"
#include "ShaderProgram.h"
#include "FrameBuffer.h"
#include "ControllerState.h"

extern vr::IVRSystem *g_pHMD;

std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = nullptr);

//-----------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
class CMainApplication
{
public:
	void Init(int argc, char *argv[]);
	void InitGL();
	void InitCompositor();

	void Shutdown();

	void RunMainLoop();
	bool handleInput();
	void handleController(vr::ETrackedControllerRole role);
	void ProcessVREvent(const vr::VREvent_t & event);
	void renderFrame();

	bool SetupStereoRenderTargets();
	void SetupCompanionWindow();
	void SetupFullScreenQuad();
	void SetupCameras();

	void RenderStereoTargets();
	void RenderCompanionWindow();
	void RenderScene(vr::Hmd_Eye nEye);

	glm::mat4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
	glm::mat4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);
	void updateDevicePoses();

	void teleportPlayer(const glm::vec3& position);

private:
	bool m_bVblank = false;
	bool m_bGlFinishHack = true;

	double m_deltaTime;

	ShaderProgram* m_shaderFullScreenQuad = nullptr;
	ShaderProgram* m_shaderRenderModel = nullptr;
	ShaderProgram* m_shaderScene = nullptr;

	vr::IVRRenderModels *m_pRenderModels = nullptr;
	std::string m_strDriver;
	std::string m_strDisplay;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	glm::mat4 m_matDeviceToRoom[vr::k_unMaxTrackedDeviceCount];

	std::map<vr::ETrackedControllerRole, ControllerState> m_controllerState;

private: // SDL bookkeeping
	SDL_Window * m_pCompanionWindow = nullptr;
	uint32_t m_nCompanionWindowWidth = 2000;
	uint32_t m_nCompanionWindowHeight = 1000;

	SDL_GLContext m_pContext = nullptr;

public: // Matrices
	glm::mat4 m_matRoomToHead;
	glm::mat4 m_matHeadToEye[2];
	glm::mat4 m_matEyeToScreen[2];
	glm::mat4 m_matWorldToRoom;
	glm::mat4 m_matRoomToEye[2];
	glm::mat4 m_matWorldToScreen[2];

private: // OpenGL bookkeeping
	float m_fNearClip;
	float m_fFarClip;

	GLuint m_unCompanionWindowVAO;
	unsigned int m_uiCompanionWindowIndexSize;

	GLuint m_unFullScreenQuadVAO = 0;
	unsigned int m_uiFullScreenQuadIndexSize = 0;

	struct VertexDataWindow
	{
		glm::vec2 position;
		glm::vec2 texCoord;

		VertexDataWindow(const glm::vec2 & pos, const glm::vec2 tex) : position(pos), texCoord(tex) {	}
	};

	std::unique_ptr<FrameBuffer> m_eyeFrameBuffer[2];

	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;

	float m_mouseLookHeading = 0.0f;
	float m_mouseLookPitch = 0.0f;
	const float c_mouseLookSensitivity = 0.001f;
	
	const float c_wasdMovementSpeed = 10.0f;

	void displayFramesPerSecond();
	uint32_t m_lastFpsTime = 0;
	int m_framesSinceLastFpsTime = 0;
};

extern CMainApplication g_app;
