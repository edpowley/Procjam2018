#include "stdafx.h"
#include "FrameBuffer.h"


GLuint FrameBuffer::createAndAttachTexture(int width, int height, int multisample, GLenum internalFormat, GLenum format, GLenum attachment)
{
	GLuint textureId;
	glGenTextures(1, &textureId);

	if (multisample)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureId);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisample, internalFormat, width, height, true);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D_MULTISAMPLE, textureId, 0);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, textureId, 0);
	}

	return textureId;
}

FrameBuffer::FrameBuffer(int width, int height, int multisample)
	: m_width(width), m_height(height), m_multisample(multisample)
{
	// Create FBO
	glGenFramebuffers(1, &m_frameBufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);

	// Create depth buffer
	m_depthTextureId = createAndAttachTexture(width, height, multisample, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT);

	// Create colour buffer
	m_colourTextureId = createAndAttachTexture(width, height, multisample, GL_RGBA8, GL_RGBA, GL_COLOR_ATTACHMENT0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		THROW(FrameBufferException() << "FrameBuffer is incomplete");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &m_frameBufferId);
	glDeleteTextures(1, &m_depthTextureId);
	glDeleteTextures(1, &m_colourTextureId);
}

void FrameBuffer::blitTo(FrameBuffer& other, GLbitfield mask, GLenum filter)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frameBufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, other.m_frameBufferId);

	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, other.m_width, other.m_height, mask, filter);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
