const char *vertex_shader_source =
"in vec2 position;"
""
"void main(void)"
"{"
"	gl_Position = vec4(position.x, position.y, 0.0, 1.0);"
"}";

const char *fragment_shader_source =
"uniform vec4 center;"
"uniform vec4 position;"
"uniform float zoom;"
"uniform int iterations;"
"uniform int periodicity;"
"uniform int bw;"
""
"vec4 color(int i)"
"{"
"	int band = iterations / 6;"
"	switch (i / band) {"
"	case 0:  return vec4(mix(0.0, 1.0, (i - 0.0 * band) / band), 0.0, 1.0, 1.0);"
"	case 1:  return vec4(1.0, 0.0, mix(1.0, 0.0, (i - 1.0 * band) / band), 1.0);"
"	case 2:  return vec4(1.0, mix(0.0, 1.0, (i - 2.0 * band) / band), 0.0, 1.0);"
"	case 3:  return vec4(mix(1.0, 0.0, (i - 3.0 * band) / band), 1.0, 0.0, 1.0);"
"	case 4:  return vec4(0.0, 1.0, mix(0.0, 1.0, (i - 4.0 * band) / band), 1.0);"
"	case 5:  return vec4(0.0, mix(1.0, 0.0, (i - 5.0 * band) / band), 1.0, 1.0);"
"	default: return vec4(0.5, 0.5, 0.5, 1.0);"
"	}"
"}"
""
"void main(void)"
"{"
"	vec4 c = position + (gl_FragCoord - center) * zoom;"
""
"	int i = 0, j = 0;"
"	vec4 z = vec4(0.0, 0.0, 0.0, 0.0);"
"	vec4 old_z = vec4(0.0, 0.0, 0.0, 0.0);"
"	while (i < iterations && length(z) <= 2.0) {"
"		z = vec4(z.x * z.x - z.y * z.y + c.x, 2 * z.x * z.y + c.y, 0.0, 0.0);"
"		i++;"
""
"		if (z == old_z) {"
"			i = iterations;"
"			break;"
"		}"
""
"		if (j++ >= periodicity) {"
"			old_z = z;"
"			j = 0;"
"		}"
"	}"
""
"	gl_FragColor = i < iterations ? bw == 0 ? color(i) : vec4(1.0, 1.0, 1.0, 1.0) : vec4(0.0, 0.0, 0.0, 0.0);"
"}";
