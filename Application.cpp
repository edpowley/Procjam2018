// GTAVive_OpenVR.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Application.h"
#include "CGLRenderModel.h"
#include "Exception.h"
#include "Conversions.h"

#if _DEBUG
#define CATCH_EXCEPTIONS_IN_MAIN 0
#else
#define CATCH_EXCEPTIONS_IN_MAIN 1
#endif

CMainApplication g_app;

vr::IVRSystem *g_pHMD = nullptr;

static bool g_bEnableVR = false;

class InitException : public Exception {};

//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError)
{
	if (!pHmd)
		return std::string();

	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, nullptr, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::Init(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		if (!stricmp(argv[i], "-novblank"))
		{
			m_bVblank = false;
		}
		else if (!stricmp(argv[i], "-noglfinishhack"))
		{
			m_bGlFinishHack = false;
		}
	}
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		THROW(InitException() << "SDL_Init failed: " << SDL_GetError());
	}

	// Loading the SteamVR Runtime
	if (g_bEnableVR)
	{
		vr::EVRInitError eError = vr::VRInitError_None;
		g_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);

		if (eError != vr::VRInitError_None)
		{
			g_pHMD = nullptr;
			THROW(InitException() << "VR_Init failed: " << vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		}


		m_pRenderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
		if (!m_pRenderModels)
		{
			g_pHMD = nullptr;
			vr::VR_Shutdown();

			THROW(InitException() << "VR_GetGenericInterface(IVRRenderModels_Version) failed: " << vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		}
	}

	int nWindowPosX = 100;
	int nWindowPosY = 100;
	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// https://forums.libsdl.org/viewtopic.php?p=36851
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

#if _DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	m_pCompanionWindow = SDL_CreateWindow("hellovr", nWindowPosX, nWindowPosY, m_nCompanionWindowWidth, m_nCompanionWindowHeight, unWindowFlags);
	if (m_pCompanionWindow == nullptr)
	{
		THROW(InitException() << "SDL_CreateWindow failed: " << SDL_GetError());
	}

	SDL_SetRelativeMouseMode(SDL_TRUE);

	m_pContext = SDL_GL_CreateContext(m_pCompanionWindow);
	if (m_pContext == nullptr)
	{
		THROW(InitException() << "SDL_GL_CreateContext failed: " << SDL_GetError());
	}

	SDL_GL_MakeCurrent(m_pCompanionWindow, m_pContext);

	glewExperimental = GL_TRUE;
	GLenum nGlewError = glewInit();
	if (nGlewError != GLEW_OK)
	{
		THROW(InitException() << "glewInit failed: " << glewGetErrorString(nGlewError));
	}
	glGetError(); // to clear the error caused deep in GLEW

	if (SDL_GL_SetSwapInterval(m_bVblank ? 1 : 0) < 0)
	{
		THROW(InitException() << "SDL_GL_SetSwapInterval failed: " << SDL_GetError());
	}


	m_strDriver = "No Driver";
	m_strDisplay = "No Display";

	m_strDriver = GetTrackedDeviceString(g_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	m_strDisplay = GetTrackedDeviceString(g_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

	std::string strWindowTitle = "hellovr - " + m_strDriver + " " + m_strDisplay;
	SDL_SetWindowTitle(m_pCompanionWindow, strWindowTitle.c_str());

	m_fNearClip = 0.1f;
	m_fFarClip = 1000.0f;

	m_matWorldToRoom = glm::mat4(1);

	// 		m_MillisecondsTimer.start(1, this);
	// 		m_SecondsTimer.start(1000, this);

	InitGL();

	InitCompositor();
}


//-----------------------------------------------------------------------------
// Purpose: Outputs the string in message to debugging output.
//          All other parameters are ignored.
//          Does not return any meaningful value or reference.
//-----------------------------------------------------------------------------
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	// Only print message 0x20070 (memory usage summary) once per second
	if (id == 0x20070)
	{
		/*static uint32_t lastTime = 0;
		if (SDL_GetTicks() - lastTime > 1000)
		{
			lastTime = SDL_GetTicks();
		}
		else*/
		{
			return;
		}
	}

	dprintf("GL Debug source=0x%X type=0x%X id=0x%X severity=0x%X\n", source, type, id, severity);
	dprintf("%s\n", message);
}


//-----------------------------------------------------------------------------
// Purpose: Initialize OpenGL. Returns true if OpenGL has been successfully
//          initialized, false if shaders could not be created.
//          If failure occurred in a module other than shaders, the function
//          may return true or throw an error. 
//-----------------------------------------------------------------------------
void CMainApplication::InitGL()
{
#if _DEBUG
	glDebugMessageCallback((GLDEBUGPROC)DebugCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

	SetupCameras();
	SetupStereoRenderTargets();
	SetupCompanionWindow();
	SetupFullScreenQuad();
	CGLRenderModel::SetupRenderModels();

	m_shaderFullScreenQuad = ShaderProgram::get("FullScreenQuad");
	m_shaderRenderModel = ShaderProgram::get("RenderModel");
	m_shaderScene = ShaderProgram::get("Scene");
}


//-----------------------------------------------------------------------------
// Purpose: Initialize Compositor. Returns true if the compositor was
//          successfully initialized, false otherwise.
//-----------------------------------------------------------------------------
void CMainApplication::InitCompositor()
{
	if (g_bEnableVR)
	{
		vr::EVRInitError peError = vr::VRInitError_None;

		if (!vr::VRCompositor())
		{
			THROW(InitException() << "Compositor initialization failed -- see log file for details");
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::Shutdown()
{
	if (g_pHMD)
	{
		vr::VR_Shutdown();
		g_pHMD = nullptr;
	}

	CGLRenderModel::Shutdown();

	if (m_pContext)
	{
#if DEBUG
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(nullptr, nullptr);
#endif

		glDeleteVertexArrays(1, &m_unCompanionWindowVAO);
		glDeleteVertexArrays(1, &m_unFullScreenQuadVAO);
	}

	if (m_pCompanionWindow)
	{
		SDL_DestroyWindow(m_pCompanionWindow);
		m_pCompanionWindow = nullptr;
	}

	SDL_Quit();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::handleInput()
{
	SDL_Event sdlEvent;
	bool bRet = false;

	while (SDL_PollEvent(&sdlEvent) != 0)
	{
		switch (sdlEvent.type)
		{
		case SDL_QUIT:
			bRet = true;
			break;

		case SDL_KEYDOWN:
			if (sdlEvent.key.keysym.sym == SDLK_ESCAPE)
			{
				bRet = true;
			}
			break;

		case SDL_MOUSEMOTION:
			if (!g_pHMD) // Only do mouse-look if VR is disabled
			{
				m_mouseLookHeading += sdlEvent.motion.xrel * c_mouseLookSensitivity;
				m_mouseLookPitch += sdlEvent.motion.yrel * c_mouseLookSensitivity;
			}
			break;
		}
	}

	glm::mat4 invHMDPose = glm::inverse(m_matRoomToHead);
	glm::vec4 forward = invHMDPose * glm::vec4(0, 0, 1, 0);
	glm::vec4 right = invHMDPose * glm::vec4(-1, 0, 0, 0);
	float delta = (float)(m_deltaTime * c_wasdMovementSpeed);

	auto keyStates = SDL_GetKeyboardState(nullptr);
	if (keyStates[SDL_SCANCODE_W])
		m_matWorldToRoom = glm::translate(glm::mat4(1), forward.xyz * delta) * m_matWorldToRoom;
	if (keyStates[SDL_SCANCODE_S])
		m_matWorldToRoom = glm::translate(glm::mat4(1), forward.xyz * -delta) * m_matWorldToRoom;
	if (keyStates[SDL_SCANCODE_D])
		m_matWorldToRoom = glm::translate(glm::mat4(1), right.xyz * delta) * m_matWorldToRoom;
	if (keyStates[SDL_SCANCODE_A])
		m_matWorldToRoom = glm::translate(glm::mat4(1), right.xyz * -delta) * m_matWorldToRoom;

	if (g_pHMD)
	{
		// Process SteamVR events
		vr::VREvent_t event;
		while (g_pHMD->PollNextEvent(&event, sizeof(event)))
		{
			ProcessVREvent(event);
		}

		// Process SteamVR controller state
		handleController(vr::TrackedControllerRole_LeftHand);
		handleController(vr::TrackedControllerRole_RightHand);
	}

	return bRet;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::handleController(vr::ETrackedControllerRole role)
{
	if (!g_pHMD) return;

	ControllerState& state = m_controllerState[role];
	vr::TrackedDeviceIndex_t deviceIndex = g_pHMD->GetTrackedDeviceIndexForControllerRole(role);
	if (deviceIndex != vr::k_unTrackedDeviceIndexInvalid)
	{
		state.update(deviceIndex, m_matDeviceToRoom[deviceIndex]);
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RunMainLoop()
{
	bool shouldQuit = false;

	SDL_StartTextInput();
	SDL_ShowCursor(SDL_DISABLE);

	uint64_t lastFrameTime = SDL_GetPerformanceCounter();

	while (!shouldQuit)
	{
		uint64_t frameTime = SDL_GetPerformanceCounter();
		m_deltaTime = (double)(frameTime - lastFrameTime) / (double)SDL_GetPerformanceFrequency();
		lastFrameTime = frameTime;

		updateDevicePoses();

		shouldQuit = handleInput();

		renderFrame();

		displayFramesPerSecond();
	}

	SDL_StopTextInput();
}


void CMainApplication::displayFramesPerSecond()
{
	m_framesSinceLastFpsTime++;
	uint32_t currentTime = SDL_GetTicks();
	if (currentTime > m_lastFpsTime + 1000)
	{
		int fps = (int)round((double)m_framesSinceLastFpsTime / (double)(currentTime - m_lastFpsTime) * 1000.0);

		std::ostringstream ss;
		ss << fps << " FPS";
		SDL_SetWindowTitle(m_pCompanionWindow, ss.str().c_str());

		m_lastFpsTime = currentTime;
		m_framesSinceLastFpsTime = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------
void CMainApplication::ProcessVREvent(const vr::VREvent_t & event)
{
	switch (event.eventType)
	{
	case vr::VREvent_TrackedDeviceActivated:
	{
		CGLRenderModel::SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
		dprintf("Device %u attached. Setting up render model.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceDeactivated:
	{
		dprintf("Device %u detached.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceUpdated:
	{
		dprintf("Device %u updated.\n", event.trackedDeviceIndex);
	}
	break;
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::renderFrame()
{
	// Update matrices
	for (int eye = 0; eye < 2; eye++)
	{
		m_matRoomToEye[eye] = m_matHeadToEye[eye] * m_matRoomToHead;
		m_matWorldToScreen[eye] = m_matEyeToScreen[eye] * m_matRoomToEye[eye] * m_matWorldToRoom;
	}

	// Render
	RenderStereoTargets();
	RenderCompanionWindow();

	// Submit frame buffers to VR headset
	if (g_pHMD)
	{
		vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)m_eyeFrameBuffer[vr::Eye_Left]->getColourTextureId(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)m_eyeFrameBuffer[vr::Eye_Right]->getColourTextureId(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
	}

	if (m_bVblank && m_bGlFinishHack)
	{
		//$ HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
		// happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
		// appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
		// 1/29/2014 mikesart
		glFinish();
	}

	// SwapWindow
	{
		SDL_GL_SwapWindow(m_pCompanionWindow);
	}

	// Clear
	{
		// We want to make sure the glFinish waits for the entire present to complete, not just the submission
		// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Flush and wait for swap.
	if (m_bVblank)
	{
		glFlush();
		glFinish();
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::SetupCameras()
{
	for (int eye = 0; eye < 2; eye++)
	{
		m_matEyeToScreen[eye] = GetHMDMatrixProjectionEye((vr::Hmd_Eye)eye);
		m_matHeadToEye[eye] = GetHMDMatrixPoseEye((vr::Hmd_Eye)eye);
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::SetupStereoRenderTargets()
{
	if (g_pHMD)
	{
		g_pHMD->GetRecommendedRenderTargetSize(&m_nRenderWidth, &m_nRenderHeight);
	}
	else
	{
		m_nRenderWidth = 1512;
		m_nRenderHeight = 1680;
	}

	m_eyeFrameBuffer[vr::Eye_Left] = std::make_unique<FrameBuffer>(m_nRenderWidth, m_nRenderHeight, 0);
	m_eyeFrameBuffer[vr::Eye_Right] = std::make_unique<FrameBuffer>(m_nRenderWidth, m_nRenderHeight, 0);

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::SetupCompanionWindow()
{
	std::vector<VertexDataWindow> vVerts;

	// left eye verts
	vVerts.push_back(VertexDataWindow(glm::vec2(-1, -1), glm::vec2(0, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(0, -1), glm::vec2(1, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(-1, 1), glm::vec2(0, 1)));
	vVerts.push_back(VertexDataWindow(glm::vec2(0, 1), glm::vec2(1, 1)));

	// right eye verts
	vVerts.push_back(VertexDataWindow(glm::vec2(0, -1), glm::vec2(0, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(1, -1), glm::vec2(1, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(0, 1), glm::vec2(0, 1)));
	vVerts.push_back(VertexDataWindow(glm::vec2(1, 1), glm::vec2(1, 1)));

	GLushort vIndices[] = { 0, 1, 3,   0, 3, 2,   4, 5, 7,   4, 7, 6 };
	m_uiCompanionWindowIndexSize = _countof(vIndices);

	glGenVertexArrays(1, &m_unCompanionWindowVAO);
	glBindVertexArray(m_unCompanionWindowVAO);

	GLuint vertexBufferId;
	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, vVerts.size() * sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW);

	GLuint indexBufferId;
	glGenBuffers(1, &indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_uiCompanionWindowIndexSize * sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, texCoord));

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::SetupFullScreenQuad()
{
	std::vector<VertexDataWindow> vVerts;

	vVerts.push_back(VertexDataWindow(glm::vec2(-1, -1), glm::vec2(0, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(+1, -1), glm::vec2(1, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(-1, +1), glm::vec2(0, 1)));
	vVerts.push_back(VertexDataWindow(glm::vec2(+1, +1), glm::vec2(1, 1)));

	GLushort vIndices[] = { 0, 1, 3,   0, 3, 2 };
	m_uiFullScreenQuadIndexSize = _countof(vIndices);

	glGenVertexArrays(1, &m_unFullScreenQuadVAO);
	glBindVertexArray(m_unFullScreenQuadVAO);

	GLuint vertexBufferId;
	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, vVerts.size() * sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW);

	GLuint indexBufferId;
	glGenBuffers(1, &indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_uiFullScreenQuadIndexSize * sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, texCoord));

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderStereoTargets()
{
	for (vr::Hmd_Eye eye : { vr::Eye_Left, vr::Eye_Right })
	{
		RenderScene(eye);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Renders a scene with respect to nEye.
//-----------------------------------------------------------------------------
void CMainApplication::RenderScene(vr::Hmd_Eye nEye)
{
	const glm::mat4& worldToScreen = m_matWorldToScreen[(int)nEye];
	glm::mat4 screenToWorld = glm::inverse(worldToScreen);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	m_eyeFrameBuffer[nEye]->bind();
	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	bool bIsInputAvailable = (g_pHMD != nullptr) && g_pHMD->IsInputAvailable();

	// Draw the scene
	m_shaderScene->use();
	m_shaderScene->u_matScreenToWorld.set(screenToWorld);
	m_shaderScene->u_eyePosition.set(glm::inverse(m_matRoomToEye[nEye] * m_matWorldToRoom) * glm::vec4(0, 0, 0, 1));
	glBindVertexArray(m_unFullScreenQuadVAO);
	glDrawElements(GL_TRIANGLES, m_uiFullScreenQuadIndexSize, GL_UNSIGNED_SHORT, nullptr);

	//m_shaderDffGeometry->use();

	// ----- Render Model rendering -----
	m_shaderRenderModel->use();

	for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!CGLRenderModel::getRenderModelForTrackedDevice(unTrackedDevice))
			continue;

		const vr::TrackedDevicePose_t & pose = m_rTrackedDevicePose[unTrackedDevice];
		if (!pose.bPoseIsValid)
			continue;

		if (!bIsInputAvailable && g_pHMD->GetTrackedDeviceClass(unTrackedDevice) == vr::TrackedDeviceClass_Controller)
			continue;

		const glm::mat4 & matDeviceToWorld = glm::inverse(m_matWorldToRoom) * m_matDeviceToRoom[unTrackedDevice];
		glm::mat4 matMVP = worldToScreen * matDeviceToWorld;
		ShaderProgram::current()->u_matObjectToScreen.set(matMVP);

		CGLRenderModel::getRenderModelForTrackedDevice(unTrackedDevice)->Draw();
	}

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderCompanionWindow()
{
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, m_nCompanionWindowWidth, m_nCompanionWindowHeight);

	glBindVertexArray(m_unCompanionWindowVAO);
	m_shaderFullScreenQuad->use();
	
	// render left eye (first half of index array )
	glBindTexture(GL_TEXTURE_2D, m_eyeFrameBuffer[vr::Eye_Left]->getColourTextureId());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDrawElements(GL_TRIANGLES, m_uiCompanionWindowIndexSize / 2, GL_UNSIGNED_SHORT, 0);

	// render right eye (second half of index array )
	glBindTexture(GL_TEXTURE_2D, m_eyeFrameBuffer[vr::Eye_Right]->getColourTextureId());
	//int n = (SDL_GetTicks() / 1000) % c_maxDepthPeels;
	//glBindTexture(GL_TEXTURE_2D, m_resolveDepthPeelBuffer[n]->getColourTextureId());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDrawElements(GL_TRIANGLES, m_uiCompanionWindowIndexSize / 2, GL_UNSIGNED_SHORT, (const void *)(uintptr_t)(m_uiCompanionWindowIndexSize));

	glBindVertexArray(0);
	glUseProgram(0);
}


//-----------------------------------------------------------------------------
// Purpose: Gets a Matrix Projection Eye with respect to nEye.
//-----------------------------------------------------------------------------
glm::mat4 CMainApplication::GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
{
	if (!g_pHMD)
	{
		return glm::perspective((float)M_PI / 180.0f * 110.0f, 1.0f, m_fNearClip, m_fFarClip);
	}

	vr::HmdMatrix44_t mat = g_pHMD->GetProjectionMatrix(nEye, m_fNearClip, m_fFarClip);
	return toGlm(mat);
}


//-----------------------------------------------------------------------------
// Purpose: Gets an HMDMatrixPoseEye with respect to nEye.
//-----------------------------------------------------------------------------
glm::mat4 CMainApplication::GetHMDMatrixPoseEye(vr::Hmd_Eye nEye)
{
	if (!g_pHMD)
	{
		float x = 0.03f;
		if (nEye == vr::Eye_Left) x = -x;
		return glm::translate(glm::mat4(1), glm::vec3(x, 0, 0));
	}

	vr::HmdMatrix34_t matEye = g_pHMD->GetEyeToHeadTransform(nEye);
	glm::mat4 matrixObj = toGlm(matEye);

	return glm::inverse(matrixObj);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::updateDevicePoses()
{
	if (g_pHMD)
	{
		vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

		for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
		{
			if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
			{
				m_matDeviceToRoom[nDevice] = toGlm(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
			}
		}

		if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
		{
			m_matRoomToHead = glm::inverse(m_matDeviceToRoom[vr::k_unTrackedDeviceIndex_Hmd]);
		}
	}
	else
	{
		m_matRoomToHead = glm::mat4(1.0f);
		m_matRoomToHead = glm::rotate(m_matRoomToHead, m_mouseLookPitch, glm::vec3(1, 0, 0));
		m_matRoomToHead = glm::rotate(m_matRoomToHead, m_mouseLookHeading, glm::vec3(0, 1, 0));
	}
}

void CMainApplication::teleportPlayer(const glm::vec3 & position)
{
	// Find the position of the player's feet in room space
	glm::vec4 currentRoomPos = glm::inverse(m_matRoomToHead) * glm::vec4(0, 0, 0, 1);
	currentRoomPos.y = 0;

	// Find the position of the player's feet in world space
	glm::vec3 currentWorldPos = glm::inverse(m_matWorldToRoom) * currentRoomPos;

	// Translation required to put the player's feet at the new position
	glm::vec3 offset = position - currentWorldPos;

	// Apply the transformation
	m_matWorldToRoom = glm::translate(m_matWorldToRoom, -offset);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
#if CATCH_EXCEPTIONS_IN_MAIN
	try
#endif
	{
		g_app.Init(argc, argv);
		g_app.RunMainLoop();
		g_app.Shutdown();

		return 0;
	}
#if CATCH_EXCEPTIONS_IN_MAIN
	catch (const std::exception& err)
	{
		MessageBoxA(NULL, err.what(), "Unhandled exception", MB_OK | MB_ICONERROR);
		return 1;
	}
#endif
}
