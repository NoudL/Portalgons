// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include <stdio.h>
#include <SDL.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/GLM/ext.hpp>

#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_opengl3.h>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#include "portalgon.h"

#include "renderer.h"


// Main code
int main(int, char**) {
	Fragment f = Fragment();
	f.createFragment();
	Polygon p = f.p;
	std::list<DrawableEdge> drawlist = f.draw();

	//Window:
	SDL_Init(SDL_INIT_EVERYTHING);

	int const width = 0.8 * 1920;
	int const height = 0.9 * 1080;

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	SDL_Window *window =
		SDL_CreateWindow("title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
			SDL_WINDOW_OPENGL //| SDL_WINDOW_FULLSCREEN_DESKTOP
		);

	SDL_GLContext sdl_gl_context = SDL_GL_CreateContext(window);

	if (glewInit() != GLEW_OK) {
		printf("Something went wrong with glew\n");
		return 1;
	}

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	//glDebugMessageCallback(gl_debug_hook, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

	SDL_GL_SetSwapInterval(1);
	glPointSize(6);
	glLineWidth(1);

	Renderer renderer;
	renderer.init();

	ImGui::CreateContext();
	ImGui::GetIO();
	ImGui_ImplSDL2_InitForOpenGL(window, sdl_gl_context);
	ImGui_ImplOpenGL3_Init();

	glViewport(0, 0, width, height);

	float camx = 0.0f;
	float camy = 0.0f;

	float scale = 100.0f;

	bool dragging = false;

	float const ZOOM_SPEED = 1.4f;

	// Main loop
	bool done = false;
	while (!done) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);

			ImGuiIO &io = ImGui::GetIO();

			if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
				continue;
			}

			switch (event.type) {
			case SDL_QUIT: {
				done = true;
			}
			case SDL_KEYDOWN: {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE: {
					done = true;
				}
				}
			} break;
			case SDL_MOUSEWHEEL: {
				if (event.wheel.y > 0) {
					scale *= ZOOM_SPEED;
				}

				if (event.wheel.y < 0) {
					scale *= 1.0f / ZOOM_SPEED;
				}
			} break;
			case SDL_MOUSEBUTTONDOWN: {
				if (event.button.button == SDL_BUTTON_LEFT) {
					dragging = true;
				}
			} break;
			case SDL_MOUSEBUTTONUP: {
				if (event.button.button == SDL_BUTTON_LEFT) {
					dragging = false;
				}
			} break;
			case SDL_MOUSEMOTION: {
				if (dragging) {
					camx += event.motion.xrel / scale;
					camy += event.motion.yrel / scale;
				}
			} break;
			}
		}

		glClearColor(1,1,1, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		renderer.new_frame();

		ImGui::ShowDemoWindow();

		if (scale > 1000.0f) {
			scale = 1000.0f;
		}
		if (scale < 1.0f) {
			scale = 1.0f;
		}

		glm::mat4 const proj =
			glm::ortho<float>(-width / 2, width / 2, -height / 2, height / 2, -1, 1);
		glm::mat4 view =
			glm::scale(glm::mat4(1.0f), glm::vec3(scale, -scale, 1.0f)) *
			glm::translate(glm::mat4(1.0f), glm::vec3(camx, camy, 1.0f));

		// Add your drawing code here.
		for each (auto e in drawlist) {
			renderer.add_line(e.edge, e.color);
		}
		renderer.add_line(Point(100.0f, 10.0f), Point(5.0f, 25.0f), 0xffffff00);
		renderer.add_line(glm::vec2(1.0f, 10.0f), glm::vec2(4.0f, 25.0f), 0xffff0000);
		renderer.add_line(glm::vec2(2.0f, 10.0f), glm::vec2(8.0f, 25.0f), 0xffff0000);
		renderer.add_line(glm::vec2(3.0f, 10.0f), glm::vec2(12.0f, 25.0f), 0xffff0000);

		renderer.draw(proj, view, ImGui::GetBackgroundDrawList());

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		SDL_GL_SwapWindow(window);
	}


	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(sdl_gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}


