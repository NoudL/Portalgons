/***
	Code largely by David Bos
*/

#define GLEW_STATIC
#include "renderer.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/GLM/glm.hpp>

static char const *vertex_shader_source = "#version 330\n"
                                          "in vec2 pos;\n"
                                          "in vec4 color0;\n"
                                          "out vec4 color;\n"
                                          "uniform mat4 transform;\n"
                                          "void main() {\n"
                                          "    color = color0;\n"
                                          "    gl_Position = transform * vec4(pos.xy, 0, 1);\n"
                                          "}\n";

static char const *fragment_shader_source = "#version 330\n"
                                            "in vec4 color;\n"
                                            "out vec4 frag_color;\n"
                                            "void main() {\n"
                                            "    frag_color = color;\n"
                                            "}\n";

static bool build_program(GLuint *prog_out, char const *vertex_shader,
                          char const *fragment_shader) {
    GLint status;
    char log[512] = { 0 };

    GLuint const vs = glCreateShader(GL_VERTEX_SHADER);
    //defer(glDeleteShader(vs));
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLsizei log_len = 0;
        glGetShaderInfoLog(vs, 512, &log_len, log);
        fputs(log, stderr);
        return false;
    }

    GLuint const fs = glCreateShader(GL_FRAGMENT_SHADER);
    //defer(glDeleteShader(fs));
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLsizei log_len = 0;
        glGetShaderInfoLog(fs, 512, &log_len, log);
        fputs(log, stderr);
        return false;
    }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLsizei log_len = 0;
        glGetProgramInfoLog(prog, 512, &log_len, log);
        fputs(log, stderr);
        glDeleteProgram(prog);
        return false;
    }

    *prog_out = prog;

    return true;
}

void Renderer::init() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    build_program(&program, vertex_shader_source, fragment_shader_source);

    pos_loc = glGetAttribLocation(program, "pos");
    color_loc = glGetAttribLocation(program, "color0");

    transform_loc = glGetUniformLocation(program, "transform");

    glGenBuffers(1, &line_vertex_buffer);
    glGenBuffers(1, &point_vertex_buffer);
}

void Renderer::new_frame() {
    line_vertex.clear();
    point_vertices.clear();
    //texts.clear();
}
void Renderer::add_line(glm::vec2 const &a, glm::vec2 const &b, uint32_t color) {
	line_vertex.push_back({ a, color });
	line_vertex.push_back({ b, color });
}

void Renderer::add_line(Point const &a, Point const &b, uint32_t color) {
    line_vertex.push_back({ glm::vec2(a.x(), a.y()), color });
    line_vertex.push_back({ glm::vec2(b.x(), b.y()), color });
}

void Renderer::add_line(Segment const &l, uint32_t color) {
    line_vertex.push_back({ glm::vec2(l.source().x(), l.source().y()), color });
    line_vertex.push_back({ glm::vec2(l.target().x(), l.target().y()), color });
}

void Renderer::add_polyline(std::vector<Point> const &points, uint32_t color) {
    for(size_t i = 1; i < points.size(); i++) {
        add_line(points[i - 1], points[i], color);
    }
}

void Renderer::add_point(Point const &p, uint32_t color) {
    point_vertices.push_back({ glm::vec2(p.x(), p.y()), color });
}

void Renderer::add_points(std::vector<Point> const &points, uint32_t color) {
    for (auto const &p : points) {
        add_point(p, color);
    }
}

//void Renderer::add_text(glm::vec2 const &p, u32 color, std::string const &text) {
//    texts.push_back({
//        .pos = p,
//        .color = color,
//        .text = text,
//    });
//}

//void Renderer::add_rect(glm::vec2 const &p, glm::vec2 const &dim, u32 color) {
//    line_vertex.push_back({ p, color });
//    line_vertex.push_back({ p + glm::vec2(dim.x, 0), color });
//
//    line_vertex.push_back({ p + glm::vec2(dim.x, 0), color });
//    line_vertex.push_back({ p + dim, color });
//
//    line_vertex.push_back({ p + dim, color });
//    line_vertex.push_back({ p + glm::vec2(0, dim.y), color });
//
//    line_vertex.push_back({ p + glm::vec2(0, dim.y), color });
//    line_vertex.push_back({ p, color });
//}

void Renderer::draw(glm::mat4 const &proj, glm::mat4 const &view, ImDrawList *draw_list) {
    glBindBuffer(GL_ARRAY_BUFFER, line_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, line_vertex.size() * sizeof(Vertex), line_vertex.data(),
                 GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, point_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, point_vertices.size() * sizeof(Vertex), point_vertices.data(),
                 GL_DYNAMIC_DRAW);

    glUseProgram(program);

    glm::mat4 const transform = proj * view;

    glUniformMatrix4fv(transform_loc, 1, GL_FALSE, (float const *)&transform);

    {
        glBindBuffer(GL_ARRAY_BUFFER, line_vertex_buffer);

        glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glEnableVertexAttribArray(pos_loc);

        glVertexAttribPointer(color_loc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
                              (void const *)8);
        glEnableVertexAttribArray(color_loc);

        glDrawArrays(GL_LINES, 0, line_vertex.size());
    }

    {
        glBindBuffer(GL_ARRAY_BUFFER, point_vertex_buffer);

        glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glEnableVertexAttribArray(pos_loc);

        glVertexAttribPointer(color_loc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
                              (void const *)8);
        glEnableVertexAttribArray(color_loc);

        glDrawArrays(GL_POINTS, 0, point_vertices.size());
    }

    assert(draw_list);

    int const width = 0.8 * 1920;
    int const height = 0.9 * 1080;

   /* for (Text const &t : texts) {
        glm::mat4 const fix = glm::scale(glm::mat4(1.0f), glm::vec3(1, -1, 1));
        glm::mat4 const mov = glm::translate(glm::mat4(1), glm::vec3(width / 2, height / 2, 1));
        glm::vec4 const p = mov * fix * view * glm::vec4(t.pos.x, t.pos.y, 0, 1);
        draw_list->AddText({ p.x, p.y }, t.color, t.text.data(), t.text.data() + t.text.size());
    }*/
}
