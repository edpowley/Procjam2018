uniform mat4 u_matScreenToWorld;
uniform vec4 u_eyePosition;
noperspective VARYING vec4 v_rayDirection;

#if _VERTEX_SHADER

layout(location = 0) in vec4 i_position;
layout(location = 1) in vec2 i_texCoords;

void main()
{
	vec4 worldPos = u_matScreenToWorld * vec4(i_position.xy, 1.0, 1.0);
	worldPos /= worldPos.w;
	v_rayDirection = worldPos - u_eyePosition;
	gl_Position = i_position;
}

#elif _FRAGMENT_SHADER

out vec4 o_colour;

void main()
{
	float rayDistanceFromOrigin = length(cross(v_rayDirection.xyz, u_eyePosition.xyz - vec3(0.0, 1.0, 0.0))) / length(v_rayDirection.xyz);
	if (rayDistanceFromOrigin <= 0.1)
		o_colour = vec4(v_rayDirection.xyz, 1.0);
	else
		o_colour = vec4(0.0, 0.0, 0.0, 1.0);
	//o_colour = vec4(u_eyePosition.xyz, 1.0);
}

#endif
