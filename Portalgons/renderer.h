/***
	Code largely by David Bos
*/


#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/GLM/glm.hpp>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Vector_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Segment_2.h>
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_opengl3.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel	K;
typedef CGAL::Vector_2<K>									Vector;
typedef CGAL::Point_2<K>									Point;
typedef CGAL::Segment_2<K>									Segment;


class Renderer {
    struct Vertex {
        glm::vec2 pos;
        uint32_t color;
    };

    std::vector<Vertex> line_vertex;
    std::vector<Vertex> point_vertices;

    /*struct Text {
        Vector pos;
        u32 color;
        std::string text;
    };*/

    //std::vector<Text> texts;

    GLuint program;
    GLint pos_loc;
    GLint color_loc;
    GLuint transform_loc;

    GLuint vao;

    GLuint line_vertex_buffer;
    GLuint point_vertex_buffer;

  public:
    void init();

    // Must be called at the start of each frame to reset the vertex buffers.
    // If you don't do this the vertex buffers will grow indefinitely.
    void new_frame();

    // color layout is 0xaabbggrr
	void add_line(glm::vec2 const &a, glm::vec2 const &b, uint32_t color);
    void add_line(Point const &a, Point const &b, uint32_t color);
    void add_line(Segment const &a, uint32_t color);
    void add_lines(std::vector<Segment> const &lines, uint32_t color) {
        for (auto line : lines) {
            add_line(line, color);
        }
    }

    void add_polyline(std::vector<Point> const &points, uint32_t color);

    void add_point(Point const &p, uint32_t color);
    void add_points(std::vector<Point> const &points, uint32_t color);

    //void add_rect(glm::vec2 const &p, glm::vec2 const &dim, uint32_t color);

    //void add_text(glm::vec2 const &p, uint32_t color, std::string const &text);

    void draw(glm::mat4 const &proj, glm::mat4 const &view, ImDrawList *draw_list);
};
