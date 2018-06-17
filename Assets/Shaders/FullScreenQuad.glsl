uniform sampler2D u_texture;
noperspective VARYING vec2 v_texCoords;

#if _VERTEX_SHADER

layout(location = 0) in vec4 i_position;
layout(location = 1) in vec2 i_texCoords;

void main()
{
	v_texCoords = i_texCoords;
	gl_Position = i_position;
}

#elif _FRAGMENT_SHADER

out vec4 o_colour;

void main()
{
	o_colour = texture(u_texture, v_texCoords);
}

#endif
