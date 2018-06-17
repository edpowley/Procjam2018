#include "stdafx.h"
#include "ShaderProgram.h"

thread_local ShaderProgram* ShaderProgram::s_current = nullptr;
std::map<std::string, std::unique_ptr<ShaderProgram>> ShaderProgram::s_cache;

static const std::string c_version = "#version 410 core\n";

static const std::string c_vertShaderHeader = c_version + "#define _VERTEX_SHADER 1\n" "#define VARYING out\n";
static const std::string c_fragShaderHeader = c_version + "#define _FRAGMENT_SHADER 1\n" "#define VARYING in\n";

ShaderProgram::ShaderProgram(const std::string& name)
	: u_matObjectToScreen(this, "u_matObjectToScreen")
	, u_lightVector(this, "u_lightVector")
	, u_eyePosition(this, "u_eyePosition")
	, u_topDepthBuffer(this, "u_topDepthBuffer")
	, u_matScreenToWorld(this, "u_matScreenToWorld")
{
	std::istringstream nameStream(name);
	std::string shaderName; nameStream >> shaderName;

	std::ostringstream code;
	std::string define;
	while (nameStream >> define)
	{
		auto equalPos = define.find('=');
		if (equalPos != std::string::npos)
		{
			code << "#define " << define.substr(0, equalPos) << " " << define.substr(equalPos + 1) << "\n";
		}
		else
		{
			code << "#define " << define << "\n";
		}
	}

	code << "#line 1 1\n";
	readAndPreprocess("Assets\\Shaders\\" + shaderName + ".glsl", code);
	std::string m_vertCode = c_vertShaderHeader + code.str();
	std::string m_fragCode = c_fragShaderHeader + code.str();

	m_programId = glCreateProgram();

	compileShader(m_vertCode.c_str(), GL_VERTEX_SHADER);
	compileShader(m_fragCode.c_str(), GL_FRAGMENT_SHADER);

	glLinkProgram(m_programId);

	GLint programSuccess = GL_TRUE;
	glGetProgramiv(m_programId, GL_LINK_STATUS, &programSuccess);
	if (programSuccess != GL_TRUE)
	{
		std::string log = getProgramInfoLog();
		glDeleteProgram(m_programId);
		m_programId = 0;
		THROW(ShaderCompileException() << "glLinkProgram failed:\n" << log);
	}

	glUseProgram(m_programId);
	glUseProgram(0);
}

ShaderProgram* ShaderProgram::get(const std::string & name)
{
	auto iter = s_cache.find(name);
	if (iter != s_cache.end())
	{
		return iter->second.get();
	}
	else
	{
		auto result = s_cache.emplace(name, new ShaderProgram(name));
		return result.first->second.get();
	}
}

ShaderProgram::~ShaderProgram()
{
	if (m_programId != 0)
	{
		glDeleteProgram(m_programId);
		m_programId = 0;
	}

	if (s_current == this)
		s_current = nullptr;
}

void ShaderProgram::use()
{
	glUseProgram(m_programId);
	s_current = this;
}

void ShaderProgram::useNull()
{
	s_current = nullptr;
}

void ShaderProgram::readAndPreprocess(const std::string & filename, std::ostream& code)
{
	std::ifstream file(filename);

	if (!file)
	{
		THROW(ShaderCompileException() << "Failed to load shader file " << filename);
	}

	std::string line;
	while (std::getline(file, line))
	{
		code << line << '\n';
	}
}

void ShaderProgram::compileShader(const char* code, GLenum type)
{
	GLuint shaderId = glCreateShader(type);
	glShaderSource(shaderId, 1, &code, nullptr);
	glCompileShader(shaderId);

	GLint success = GL_FALSE;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
	if (success != GL_TRUE)
	{
		std::string log = getShaderInfoLog(shaderId);
		glDeleteShader(shaderId);
		THROW(ShaderCompileException() << "glCompileShader failed:\n" << log);
	}

	glAttachShader(m_programId, shaderId);
	glDeleteShader(shaderId); // the program hangs onto this once it's attached
}

std::string ShaderProgram::getShaderInfoLog(GLuint shaderId)
{
	GLint logSize = 0;
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logSize);
	char* logBuffer = new char[logSize + 1];
	glGetShaderInfoLog(shaderId, logSize + 1, nullptr, logBuffer);
	std::string log(logBuffer);
	delete[] logBuffer;
	return log;
}

std::string ShaderProgram::getProgramInfoLog()
{
	GLint logSize = 0;
	glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &logSize);
	char* logBuffer = new char[logSize + 1];
	glGetProgramInfoLog(m_programId, logSize + 1, nullptr, logBuffer);
	std::string log(logBuffer);
	delete[] logBuffer;
	return log;
}
