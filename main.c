#include <stdlib.h>
#include <unistd.h>
#include "SDL.h"
#include "palette.h"

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

void draw_mandelbrot(SDL_Renderer *renderer, int bw, int width, int height, int iterations, int periodicity,
		     double pos_x, double pos_y, double zoom)
{
	const double center_x = width / 2.0, center_y = height / 2.0;
	for (int screen_y = 0; screen_y < height; screen_y++) {
		for (int screen_x = 0; screen_x < width; screen_x++) {
			const double cx = pos_x + (screen_x - center_x) * zoom;
			const double cy = pos_y + (screen_y - center_y) * zoom;

			int i = 0, j = 0;
			double x = 0.0, y = 0.0, old_x = 0.0, old_y = 0.0;
			while (i < iterations && x * x + y * y <= 4) {
				double tmp_x = x * x - y * y + cx;
				double tmp_y = 2.0 * x * y + cy;
				x = tmp_x, y = tmp_y;
				i++;

				if (x == old_x && y == old_y) {
					i = iterations;
					break;
				}

				if (j++ >= periodicity)
					old_x = x, old_y = y, j = 0;
			}

			SDL_Color color = i < iterations
					? bw ? white : palette[i * palette_len / iterations]
					: black;
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
			SDL_RenderDrawPoint(renderer, screen_x, screen_y);
		}
	}

	SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
	int bw = 0, verbose = 0, iterations = ITERATIONS, periodicity = PERIODICITY, width = WIDTH, height = HEIGHT;
	double pos_x = INITIAL_X, pos_y = INITIAL_Y, zoom = max(SCALE_WIDTH / width, SCALE_HEIGHT / height);

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

	SDL_Window *window = SDL_CreateWindow("mandelbrot", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
					      width, height, SDL_WINDOW_RESIZABLE);
	if (!window) {
		SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		SDL_Log("SDL_CreateRenderer: %s\n", SDL_GetError());
		return 1;
	}

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
					pos_y -= height / 8.0 * zoom;
					redraw = 1;
					break;
				case SDLK_s: case SDLK_DOWN:
					pos_y += height / 8.0 * zoom;
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
					pos_y += (event.button.y - height / 2.0) * zoom;
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
				SDL_Log("draw_mandelbrot\npos_x = %.20g\npos_y = %.20g\nzoom = %.20g\n", pos_x, pos_y,
					zoom);
			Uint64 start = SDL_GetPerformanceCounter();
			draw_mandelbrot(renderer, bw, width, height, iterations, periodicity, pos_x, pos_y, zoom);
			Uint64 now = SDL_GetPerformanceCounter();
			if (verbose)
				SDL_Log("took %f seconds\n", (double) (now - start) /SDL_GetPerformanceFrequency());
			redraw = 0;
		}

		SDL_Delay(delay);
	}

exit:
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	return 0;
}
