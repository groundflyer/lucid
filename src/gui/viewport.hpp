// -*- C++ -*-
// viewport.hpp
//

#pragma once

#include <base/types.hpp>
#include <image/image.hpp>

#include <GLFW/glfw3.h>

// ImGui
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <cstdio>
#include <type_traits>

namespace lucid
{
namespace detail
{
static inline int _gl_initialized = false;

template <typename ResizeReaction, typename KeyReaction, typename MouseReaction>
class _Viewport
{
    static inline GLFWwindow* window = nullptr;
    static inline Vec2i       window_size{1280, 768};

    static inline unsigned texture = -1u;
    static inline Vec2i    texture_size{640, 480};

    static inline ResizeReaction resize_reaction;
    static inline KeyReaction    key_reaction;
    static inline MouseReaction  mouse_reaction;

    static void
    resize_callback(GLFWwindow*, int width, int height) noexcept
    {
        window_size = Vec2i(width, height);
        glViewport(0, 0, width, height);
    }

    static void
    key_callback(GLFWwindow*, int key, int /*scancode*/, int action, int mods) noexcept
    {
        key_reaction(key, action, mods);
    }

    static void
    mouse_button_callback(GLFWwindow*, int button, int action, int mods) noexcept
    {
        mouse_reaction(button, action == GLFW_PRESS, mods);
    }

    static void
    cursor_position_callback(GLFWwindow*, double xpos, double ypos) noexcept
    {
        if constexpr(std::is_void_v<std::invoke_result_t<MouseReaction, Vec2>>)
            mouse_reaction(Vec2{xpos, ypos});
        else
            reload_img(mouse_reaction(Vec2{xpos, ypos}));
    }

    _Viewport()                 = delete;
    _Viewport(const _Viewport&) = delete;
    _Viewport(_Viewport&&)      = delete;
    _Viewport&
    operator=(const _Viewport&) = delete;
    ~_Viewport()                = delete;

    static void
    init(const Vec2u&     _res,
         ResizeReaction&& _resize_reaction,
         KeyReaction&&    _key_reaction,
         MouseReaction&&  _mouse_reaction)
    {
        window_size        = Vec2i{_res};
        const auto& [w, h] = window_size;
        window             = glfwCreateWindow(w, h, "Lucid", nullptr, nullptr);

        if(!window)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create window");
        }

        resize_reaction = _resize_reaction;
        mouse_reaction  = _mouse_reaction;
        key_reaction    = _key_reaction;

        glfwSwapInterval(1);
        glfwMakeContextCurrent(window);
        // glfwSetFramebufferSizeCallback(window, resize_callback);
        // glfwSetKeyCallback(window, key_callback);
        // glfwSetMouseButtonCallback(window, mouse_button_callback);
        // glfwSetCursorPosCallback(window, cursor_position_callback);

        // texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        std::printf("Context created\n");
        // ImGuiIO& io = ImGui::GetIO();
        // (void)io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();
        std::printf("Style applied\n");

        // ImGuiStyle& style = ImGui::GetStyle();
        // if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        // {
        //     style.WindowRounding              = 0.0f;
        //     style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        // }

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");
    }

    static const Vec2i&
    get_res() noexcept
    {
        return window_size;
    }

    template <typename Format>
    static void
    load_img(const lucid::ScanlineImage<Format, 4>& img) noexcept
    {
        const constexpr auto type_flag =
            std::is_same_v<Format, unsigned char> ?
                GL_UNSIGNED_BYTE :
                (std::is_same_v<Format, float> ? GL_FLOAT : GL_UNSIGNED_INT);
        texture_size               = Vec2i{img.res()};
        const auto [width, height] = texture_size;
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
        glfwPollEvents();

        // const auto& [width, height] = texture_size;

        std::printf("foo\n");
        ImGui_ImplOpenGL3_NewFrame();
        std::printf("gl3 newframe\n");
        ImGui_ImplGlfw_NewFrame();
        std::printf("glfw newframe\n");

        ImGui::NewFrame();
        std::printf("imgui new frame\n");
        ImGui::Begin("Viewport texture test");
        ImGui::Text("This is some useful text.");
        // ImGui::Text("size = %d x %d", width, height);
        // // ImGui::Image(static_cast<void*>(&texture), ImVec2(width, height));
        ImGui::End();

        ImGui::Render();
        auto& [width, height] = window_size;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        std::printf("Imgui render\n");
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        std::printf("renderdrawdata\n");

        glfwSwapBuffers(window);

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
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
        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

  public:
    struct Handler
    {
        Handler() noexcept      = delete;
        Handler(const Handler&) = delete;
        Handler(Handler&&)      = delete;
        Handler&
        operator=(const Handler&) = delete;
        Handler&
        operator=(Handler&&) = delete;

        Handler(const Vec2u& res, ResizeReaction&& ra, KeyReaction&& ka, MouseReaction&& ma)
        {
            _Viewport::init(res, std::move(ra), std::move(ka), std::move(ma));
        }
        ~Handler() noexcept { _Viewport::cleanup(); }

        const Vec2i&
        get_res() const noexcept
        {
            return _Viewport::get_res();
        }

        template <typename Format>
        void
        load_img(const lucid::ScanlineImage<Format, 4>& img) const noexcept
        {
            _Viewport::load_img(img);
        }

        template <typename Format>
        void
        reload_img(const lucid::ScanlineImage<Format, 4>& img) const
        {
            _Viewport::reload_img(img);
        }

        bool
        active() const noexcept
        {
            return _Viewport::active();
        }

        void
        draw() const noexcept
        {
            _Viewport::draw();
        }

        void
        check_errors() const
        {
            _Viewport::check_errors();
        }
    };
};
} // namespace detail

template <typename... Actions>
auto
make_viewport(const Vec2u& res, Actions&&... actions)
{
    if(!detail::_gl_initialized)
    {
        detail::_gl_initialized = glfwInit();
        if(!detail::_gl_initialized) throw detail::_gl_initialized;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHintString(GLFW_X11_CLASS_NAME, "Lucid");
        glfwWindowHintString(GLFW_X11_INSTANCE_NAME, "Lucid");
    }

    return
        typename detail::_Viewport<std::decay_t<Actions>...>::Handler(res, std::move(actions)...);
}
} // namespace lucid
