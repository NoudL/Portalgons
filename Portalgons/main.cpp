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
#include "Raytracer.h"
#include <CGAL/Gmpq.h>

#include "parser.h"
#include <map>
#include <sstream>

// Main code
int main(int, char**) {

	/*
		SETUP
	*/
	Portalgon p = createPortalgonFromGraphml("Salzburg/fpg/fpg-poly_0000000010.graphml");
	//Portalgon p = createPortalgonFromGraphml("Salzburg/spg-a-2opt/spg-a-poly_0002000_1.graphml");
	//Portalgon p = createPortalgonFromIpe("ipe/test4.ipe");
	//Portalgon p = createPortalgonFromIpe("ipe/test3.ipe");
	//Portalgon p = createPortalgonFromIpe("ipe/triangulation_test.ipe");
	std::vector<DrawableEdge> drawlist = p.draw();
	std::vector<DrawableEdge> drawCDTlist;
	std::vector<PathSegment> raysegs;
	int amount_of_steps = 50;
	double step = PI * 2 / amount_of_steps;
	float stepsize = 1.0f;
	Raytracer r = Raytracer(320, 576, 10.0, stepsize);
	for (int i = 0; i < amount_of_steps; i++)
	{
		double angle =  step * i - 0.3;

		PathSegment rayseg = r.castRaySegment(p, Direction(cos(angle), sin(angle)));
		raysegs.push_back(rayseg);
	}
	/*
		SDL stuff
	*/
	//Window:
	SDL_Init(SDL_INIT_EVERYTHING); 

	int const width = int(0.8 * 1920);
	int const height = int(0.9 * 1080);

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

	float camx = -200.0f;
	float camy = 500.0f;

	float scale = 1.0f;

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
		
		
		//User interface
		ImGui::Begin("Config");

		static bool invertYbool = true;
		ImGui::Checkbox("Invert Y Axis", &invertYbool);
		static bool drawCDT = false;
		ImGui::Checkbox("Draw Constrained Delaunay Triangulation", &drawCDT);
		std::vector<PathSegment> newraysegs;
		if (ImGui::Button("Propagate"))
		{
			for each (PathSegment rs in raysegs)
			{
				/*for each (PathSegment rs2 in raysegs) {
					r.collideRaySegs(&rs, &rs2);
				}*/
				if (!rs.closed) {
					std::vector<PathSegment> newsegments = r.expandRay(rs, p);
					newraysegs.insert(newraysegs.end(), newsegments.begin(), newsegments.end());
				}
				else {
					newraysegs.push_back(rs);
				}
				
			}
			raysegs = newraysegs;
		}

		ImGui::End();

		int invertY = (invertYbool) ? -1 : 1;
		if (scale > 1000.0f) {
			scale = 1000.0f;
		}
		if (scale < 0.5f) {
			scale = 0.5f;
		}

		//Render the geometry
		glm::mat4 const proj =
			glm::ortho<float>(-width / 2, width / 2, height / 2, -height / 2, -1, 1);
		glm::mat4 view =
			glm::scale(glm::mat4(1.0f), glm::vec3(scale, invertY * scale, 1.0f)) *
			glm::translate(glm::mat4(1.0f), glm::vec3(camx, invertY * camy, 1.0f));

		if (drawCDT) {
			drawCDTlist = p.drawCDTs();
		}
		else {
			drawCDTlist = {};
		}

		// Add your drawing code here.
		for each (auto e in drawCDTlist) {
			renderer.add_line(e.edge, 0x0000FF);
		}
		for each (auto e in drawlist) {
			renderer.add_line(e.edge, e.color);
		}
		for each (auto e in raysegs) {
			renderer.add_line(e.segment, 0x0);
		}

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


