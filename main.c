#include <stdlib.h>
#include <unistd.h>
#include <GL/glew.h>
#include "SDL.h"
#include "shaders.h"

#define ITERATIONS 1024
#define PERIODICITY 20
#define WIDTH 640
#define HEIGHT 480
#define INITIAL_X -0.75
#define INITIAL_Y 0.0
#define SCALE_WIDTH 3.5
#define SCALE_HEIGHT 2.0
#define ZOOM_FACTOR 1.25

double max(double a, double b)
{
	return a > b ? a : b;
}

GLuint load_shader(GLenum type, const char *source)
{
	GLuint shader = glCreateShader(type);
	char *sources[2] = {"#version 150\n", source};
	glShaderSource(shader, 2, sources, NULL);
	glCompileShader(shader);

	GLint compiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE) {
		GLint len = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

		char *msg = malloc(len);
		glGetShaderInfoLog(shader, len, &len, msg);
		SDL_Log("load_shader: %s\n", msg);
		free(msg);
		
		glDeleteShader(shader);

		return 0;
	}

	return shader;
}

void draw_mandelbrot(SDL_Window *window, GLuint program, int iterations, float pos_x, float pos_y, float zoom)
{
	glUniform1i(glGetUniformLocation(program, "iterations"), iterations);
	glUniform4f(glGetUniformLocation(program, "position"), pos_x, pos_y, 0.0, 0.0);
	glUniform1f(glGetUniformLocation(program, "zoom"), zoom);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	SDL_GL_SwapWindow(window);
}

int main(int argc, char *argv[])
{
	int bw = 0, verbose = 0, iterations = ITERATIONS, periodicity = PERIODICITY, width = WIDTH, height = HEIGHT;
	float pos_x = INITIAL_X, pos_y = INITIAL_Y, zoom = max(SCALE_WIDTH / width, SCALE_HEIGHT / height);

	int c;
	while ((c = getopt(argc, argv, "bvi:p:w:h:x:y:z:")) != -1) {
		switch (c) {
		case 'b': bw = 1; break;
		case 'v': verbose = 1; break;
		case 'i': iterations = atoi(optarg); break;
		case 'p': periodicity = atoi(optarg); break;
		case 'w': width = atoi(optarg); break;
		case 'h': height = atoi(optarg); break;
		case 'x': pos_x = atof(optarg); break;
		case 'y': pos_y = atof(optarg); break;
		case 'z': zoom = atof(optarg); break;
		case '?': return 1;
		}
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
		SDL_Log("SDL_Init: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) ||
	    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2) ||
	    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)) {
		SDL_Log("SDL_SetAttribute: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("mandelbrot", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
					      width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!window) {
		SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
		return 1;
	}
	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (!context) {
		SDL_Log("SDL_GL_CreateContext: %s\n", SDL_GetError());
		return 1;
	}

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		SDL_Log("glewInit: %s\n", glewGetErrorString(err));
		return 1;
	}

	if (SDL_GL_SetSwapInterval(1)) {
		SDL_Log("SDL_GL_SetSwapInterval: %s\n", SDL_GetError());
		return 1;
	}

	GLuint vertex_shader = load_shader(GL_VERTEX_SHADER, vertex_shader_source);
	GLuint fragment_shader = load_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
	if (!vertex_shader || !fragment_shader) {
		return 1;
	}

	GLuint program = glCreateProgram();

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);
	glUseProgram(program);

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	const GLfloat rectangle[4][2] = {
		{-1.0, -1.0},
		{ 1.0, -1.0},
		{-1.0,  1.0},
		{ 1.0,  1.0},
	};

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle), rectangle, GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glUniform1i(glGetUniformLocation(program, "bw"), bw);
	glUniform4f(glGetUniformLocation(program, "center"), width / 2.0, height / 2.0, 0.0, 0.0);
	glUniform4f(glGetUniformLocation(program, "position"), pos_x, pos_y, 0.0, 0.0);
	glUniform1f(glGetUniformLocation(program, "zoom"), zoom);
	glUniform1i(glGetUniformLocation(program, "iterations"), iterations);
	glUniform1i(glGetUniformLocation(program, "periodicity"), periodicity);

	SDL_DisplayMode mode;
	if (SDL_GetWindowDisplayMode(window, &mode)) {
		SDL_Log("SDL_GetWindowDisplayMode: %s\n", SDL_GetError());
		return 1;
	}
	int delay = 1000 / mode.refresh_rate;

	for (;;) {
		int redraw = 0;

		for (SDL_Event event; SDL_PollEvent(&event);) {
			switch (event.type) {
			case SDL_QUIT:
				goto exit;
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
				case SDL_WINDOWEVENT_EXPOSED:
					redraw = 1;
					break;
				case SDL_WINDOWEVENT_RESIZED:
					width = event.window.data1;
					height = event.window.data2;
					glViewport(0, 0, width, height);
					glUniform4f(glGetUniformLocation(program, "center"), width / 2.0, height / 2.0, 0.0, 0.0);
					break;
				}
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_d: case SDLK_RIGHT:
					pos_x += width / 8.0 * zoom;
					redraw = 1;
					break;
				case SDLK_a: case SDLK_LEFT:
					pos_x -= width / 8.0 * zoom;
					redraw = 1;
					break;
				case SDLK_w: case SDLK_UP:
					pos_y += height / 8.0 * zoom;
					redraw = 1;
					break;
				case SDLK_s: case SDLK_DOWN:
					pos_y -= height / 8.0 * zoom;
					redraw = 1;
					break;
				case SDLK_z:
					zoom /= ZOOM_FACTOR;
					redraw = 1;
					break;
				case SDLK_x:
					zoom *= ZOOM_FACTOR;
					redraw = 1;
					break;
				case SDLK_r:
					pos_x = INITIAL_X, pos_y = INITIAL_Y;
					zoom = max(SCALE_WIDTH / width, SCALE_HEIGHT / height);
					redraw = 1;
					break;
				case SDLK_b:
					bw = !bw;
					glUniform1i(glGetUniformLocation(program, "bw"), bw);
					redraw = 1;
					break;
				case SDLK_COMMA:
					if (iterations > 1)
					        iterations /= 2;
					redraw = 1;
					break;
				case SDLK_PERIOD:
					iterations *= 2;
					redraw = 1;
					break;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT) {
					pos_x += (event.button.x - width  / 2.0) * zoom;
					pos_y -= (event.button.y - height / 2.0) * zoom;
					redraw = 1;
				}
				break;
			case SDL_MOUSEWHEEL:
				if (event.wheel.y > 0)
					zoom /= ZOOM_FACTOR;
				else
					zoom *= ZOOM_FACTOR;
				redraw = 1;
				break;
			}
		}

		if (redraw) {
			if (verbose)
				SDL_Log("draw_mandelbrot:\npos_x = %.20g\npos_y = %.20g\nzoom = %.20g\n", pos_x, pos_y,
					zoom);
			Uint64 start = SDL_GetPerformanceCounter();
			draw_mandelbrot(window, program, iterations, pos_x, pos_y, zoom);
			Uint64 now = SDL_GetPerformanceCounter();
			if (verbose)
				SDL_Log("took %f seconds\n", (double) (now - start) /SDL_GetPerformanceFrequency());
			redraw = 0;
		}

		SDL_Delay(delay);
	}

exit:
	glDeleteProgram(program);

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);

	return 0;
}
