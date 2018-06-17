#pragma once

#include "NonCopyable.h"
#include "Exception.h"
#include "ShaderUniform.h"

class ShaderCompileException : public Exception {};

class ShaderProgram : public NonCopyable
{
public:
	static ShaderProgram* get(const std::string& name);

	~ShaderProgram();

	void use();
	static void useNull();
	static ShaderProgram* current() { return s_current; }

	GLuint getId() { return m_programId; }

	ShaderUniform<glm::mat4> u_matObjectToScreen, u_matScreenToWorld;
	ShaderUniform<glm::vec4> u_lightVector, u_eyePosition;
	ShaderUniform<int> u_topDepthBuffer;

private:
	ShaderProgram(const std::string& name);

	static std::map<std::string, std::unique_ptr<ShaderProgram>> s_cache;

	static thread_local ShaderProgram* s_current;

	GLuint m_programId = 0;

	static void readAndPreprocess(const std::string& filename, std::ostream& code);
	void compileShader(const char* code, GLenum type);
	std::string getShaderInfoLog(GLuint shaderId);
	std::string getProgramInfoLog();
};
