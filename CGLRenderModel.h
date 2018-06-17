#pragma once

class CGLRenderModel
{
public:
	CGLRenderModel(const std::string & sRenderModelName);
	~CGLRenderModel();

	void Init(const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture);
	void Cleanup();
	void Draw();
	const std::string & GetName() const { return m_sModelName; }

	static void SetupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	static CGLRenderModel *FindOrLoadRenderModel(const char *pchRenderModelName);

	static void SetupRenderModels();
	static void Shutdown();

	static CGLRenderModel *getRenderModelForTrackedDevice(uint32_t device) { return s_rTrackedDeviceToRenderModel[device]; }

private:
	GLuint m_glVertBuffer;
	GLuint m_glIndexBuffer;
	GLuint m_glVertArray;
	GLuint m_glTexture;
	GLsizei m_unVertexCount;
	std::string m_sModelName;

	static std::vector< CGLRenderModel * > s_vecRenderModels;
	static CGLRenderModel *s_rTrackedDeviceToRenderModel[vr::k_unMaxTrackedDeviceCount];
};
