#pragma once

#include "NonCopyable.h"
#include "Exception.h"

class FrameBufferException : public Exception {};

class FrameBuffer : public NonCopyable
{
public:
	FrameBuffer(int width, int height, int multisample);
	~FrameBuffer();

	void bind() { glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId); }

	GLuint getDepthTextureId() { return m_depthTextureId; }
	GLuint getColourTextureId() { return m_colourTextureId; }

	void blitTo(FrameBuffer& other, GLbitfield mask = GL_COLOR_BUFFER_BIT, GLenum filter = GL_NEAREST);

private:
	int m_width, m_height, m_multisample;

	GLuint m_frameBufferId = 0;
	GLuint m_depthTextureId = 0, m_colourTextureId = 0;

	GLuint createAndAttachTexture(int width, int height, int multisample, GLenum internalFormat, GLenum format, GLenum attachment);
};

