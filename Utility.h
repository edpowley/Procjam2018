#pragma once


// https://stackoverflow.com/a/3009806
struct caseInsensitiveStringComparer
{
	bool operator()(const std::string &lhs, const std::string &rhs) const
	{
		return stricmp(lhs.c_str(), rhs.c_str()) < 0;
	}
};

#define GLM_OSTREAM_SHIFT_OPERATOR(TYPE) \
	inline std::ostream& operator<<(std::ostream& out, const TYPE& value) { return out << glm::to_string(value); }

GLM_OSTREAM_SHIFT_OPERATOR(glm::vec2)
GLM_OSTREAM_SHIFT_OPERATOR(glm::vec3)

inline bool approx(float a, float b, float epsilon = 1.0e-5f)
{
	return fabsf(b - a) <= epsilon;
}

inline bool approx(const glm::vec3& a, const glm::vec3& b)
{
	glm::vec3 diff = b - a;
	return approx(a.x, b.x) && approx(a.y, b.y) && approx(a.z, b.z);
}

inline std::string getFileNameWithoutExtension(const std::string& path)
{
	auto dotPos = path.rfind('.');
	auto slashPos = path.rfind('\\');
	return path.substr(slashPos + 1, dotPos - slashPos - 1);
}

inline bool startsWith(const std::string& string, const std::string& prefix)
{
	if (prefix.length() > string.length()) return false;
	return std::equal(prefix.begin(), prefix.end(), string.begin());
}

inline bool endsWith(const std::string& string, const std::string& suffix)
{
	if (suffix.length() > string.length()) return false;
	return std::equal(suffix.rbegin(), suffix.rend(), string.rbegin());
}

