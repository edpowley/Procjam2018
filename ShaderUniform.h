#pragma once

class ShaderProgram;

template<typename T>
class ShaderUniform
{
	static const GLint c_locationUnknown = -2;

public:
	ShaderUniform(ShaderProgram* program, const char* name) : m_program(program), m_name(name) {}

	void set(const T& value)
	{
		if (m_location == c_locationUnknown)
			m_location = glGetUniformLocation(m_program->getId(), m_name);

		if (m_location >= 0)
			doSet(value);
/*#if _DEBUG
		else
			dprintf("Warning: attempted to set non-existant uniform %s\n", m_name);
#endif*/
	}

protected:
	ShaderProgram * m_program;
	const char* m_name;
	GLint m_location = c_locationUnknown;

	void doSet(const T& value);
};

inline void ShaderUniform<glm::vec4>::doSet(const glm::vec4& value)
{
	glUniform4fv(m_location, 1, glm::value_ptr(value));
}

inline void ShaderUniform<glm::mat4>::doSet(const glm::mat4& value)
{
	glUniformMatrix4fv(m_location, 1, GL_FALSE, glm::value_ptr(value));
}

inline void ShaderUniform<int>::doSet(const int& value)
{
	glUniform1i(m_location, value);
}
