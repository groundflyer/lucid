// -*- C++ -*-
// viewport.hpp
//

#pragma once

#include <base/types.hpp>
#include <image/image.hpp>

#include <GLFW/glfw3.h>

namespace lucid
{

template <typename MouseAction>
class _Viewport
{
    static const constexpr char* vertex_shader_src = "#version 330 core\n"
                                                     "layout (location = 0) in vec3 inP;"
                                                     "layout (location = 1) in vec2 inUV;"
                                                     "out vec2 UV;"
                                                     "void main(){"
                                                     "gl_Position = vec4(inP, 1.0);"
                                                     "UV = inUV;}";

    static const constexpr char* fragment_shader_src = "#version 330 core\n"
                                                       "out vec4 Cf;"
                                                       "in vec2 UV;"
                                                       "uniform sampler2D img;"
                                                       "void main(){Cf = texture(img, UV);}";

    static const constexpr float vertex_data[] = {
        // positions    // texture coordinates
        -1.f, 1.f, 0, 0, 0, 1.f, 1.f, 0, 1.f, 0, 1.f, -1.f, 0, 1.f, 1.f, -1.f, -1.f, 0, 0, 1.f};

    static const constexpr unsigned indices[] = {
        0,
        1,
        2, // first triangle
        2,
        3,
        0 // second triangle
    };

    static inline GLFWwindow* window;
    static inline unsigned    shader_program;
    static inline unsigned    VBO;
    static inline unsigned    VAO;
    static inline unsigned    EBO;
    static inline unsigned    texture;
    static inline Vec2i       res;

    static inline MouseAction mouse_action;

    static void
    resize_callback(GLFWwindow*, int width, int height) noexcept
    {
        res = Vec2i(width, height);
        glViewport(0, 0, width, height);
    }

    static void
    mouse_button_callback(GLFWwindow*, int button, int action, int mods) noexcept
    {
        mouse_action(button, action == GLFW_PRESS, mods);
    }

    static void
    cursor_position_callback(GLFWwindow*, double xpos, double ypos) noexcept
    {
        mouse_action(Vec2{xpos, ypos});
    }

  public:
    _Viewport()                 = delete;
    _Viewport(const _Viewport&) = delete;
    _Viewport(_Viewport&&)      = delete;
    _Viewport&
    operator=(const _Viewport&) = delete;
    ~_Viewport()                = delete;

    static void
    init(const Vec2u& _res, MouseAction&& _mouse_action)
    {
        const int status = glfwInit();
        if(!status) throw status;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        const auto [iwidth, iheight] = _res;
        window                       = glfwCreateWindow(
            static_cast<int>(iwidth), static_cast<int>(iheight), "Lucid", nullptr, nullptr);

        if(!window)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create window");
        }

        mouse_action = _mouse_action;

        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, resize_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);

        auto& [width, height] = res;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        int    shader_status;
        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_src, nullptr);
        glCompileShader(vertex_shader);
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_status);
        if(!shader_status)
        {
            char info_log[512];
            glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
            glfwTerminate();
            throw std::runtime_error(info_log);
        }

        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_src, nullptr);
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);
        if(!shader_status)
        {
            char info_log[512];
            glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
            glfwTerminate();
            throw std::runtime_error(info_log);
        }

        shader_program = glCreateProgram();
        glAttachShader(shader_program, vertex_shader);
        glAttachShader(shader_program, fragment_shader);
        glLinkProgram(shader_program);
        glGetProgramiv(shader_program, GL_LINK_STATUS, &shader_status);
        if(!shader_status)
        {
            char info_log[512];
            glGetProgramInfoLog(shader_program, 512, nullptr, info_log);
            glfwTerminate();
            throw std::runtime_error(info_log);
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        // glGenBuffers(1, &PBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), static_cast<void*>(0));
        glEnableVertexAttribArray(0);

        // uv
        glVertexAttribPointer(1,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              5 * sizeof(float),
                              reinterpret_cast<void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    static const Vec2i&
    get_res() noexcept
    {
        return res;
    }

    template <typename Format>
    static void
    load_img(const lucid::ScanlineImage<Format, 4>& img) noexcept
    {
        const constexpr auto type_flag =
            std::is_same_v<Format, unsigned char> ?
                GL_UNSIGNED_BYTE :
                (std::is_same_v<Format, float> ? GL_FLOAT : GL_UNSIGNED_INT);
        const auto [width, height] = img.res();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, type_flag, img.data());
    }

    template <typename Format>
    static void
    reload_img(const lucid::ScanlineImage<Format, 4>& img)
    {
        const constexpr auto type_flag =
            std::is_same_v<Format, unsigned char> ?
                GL_UNSIGNED_BYTE :
                (std::is_same_v<Format, float> ? GL_FLOAT : GL_UNSIGNED_INT);
        const auto [width, height] = img.res();
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, type_flag, img.data());
        auto glerror = glGetError();
        if(glerror) { throw glerror; }
    }

    static bool
    active() noexcept
    {
        return !glfwWindowShouldClose(window);
    }

    static void
    draw() noexcept
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
    }

    static void
    check_errors()
    {
        const GLenum glerror = glGetError();
        if(glerror) { throw glerror; }
    }

    static void
    cleanup() noexcept
    {
        if(window) glfwDestroyWindow(window);

        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        // glDeleteBuffers(1, &PBO);

        glfwTerminate();
    }
};
} // namespace lucid
