#pragma once

inline glm::mat4 toGlm(const vr::HmdMatrix34_t &mat)
{
	glm::mat4 matrixObj(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0,
		mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0,
		mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0,
		mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f
	);
	return matrixObj;
}

inline glm::mat4 toGlm(const vr::HmdMatrix44_t &mat)
{
	return glm::mat4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}

inline glm::vec2 toGlm(const vr::VRControllerAxis_t& axis)
{
	return glm::vec2(axis.x, axis.y);
}
