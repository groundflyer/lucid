// -*- C++ -*-
// lucid.cpp

#include <ray_traversal/primitives/generic.hpp>
#include <cameras/perspective.hpp>
#include <cameras/utils.hpp>
#include <image/io.hpp>
#include <utils/seq.hpp>
#include <utils/logging.hpp>
#include <utils/timer.hpp>
#include <base/rng.hpp>

#include <GLFW/glfw3.h>

#include <string>
#include <stdexcept>


using namespace lucid;

namespace vp
{
    static const constexpr char* vertex_shader_src =
        "#version 330 core\n"
        "layout (location = 0) in vec3 inP;"
        "layout (location = 1) in vec2 inUV;"
        "out vec2 UV;"
        "void main(){"
        "gl_Position = vec4(inP, 1.0);"
        "UV = inUV;}";

    static const constexpr char* fragment_shader_src =
        "#version 330 core\n"
        "out vec4 Cf;"
        "in vec2 UV;"
        "uniform sampler2D img;"
        "void main(){Cf = texture(img, UV);}";
        // "void main(){Cf = vec4(UV, 0, 1);}";

    static const constexpr float vertex_data[] = {
        // positions    // texture coordinates
        -1.f,  1.f, 0,       0, 0,
        1.f, 1.f, 0,       1.f, 0,
        1.f, -1.f, 0,      1.f, 1.f,
        -1.f, -1.f, 0,      0, 1.f
    };

    static const constexpr unsigned indices[] = {
        0, 1, 2, // first triangle
        2, 3, 0  // second triangle
    };

    static GLFWwindow* window;

    static unsigned shader_program;
    static unsigned VBO;
    static unsigned VAO;
    static unsigned EBO;
    static unsigned texture;

    static lucid::Vec2u res;

    static void
    resize(GLFWwindow*, int width, int height) noexcept
    {
        res = Vec2u(width, height);
        glViewport(0, 0, width, height);
    }

    static void
    init()
    {
        if(!glfwInit())
            throw std::runtime_error("Failed to initialize GLFW");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(640, 640, "Lucid", nullptr, nullptr);

        if(!window)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create window");
        }

        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, resize);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        res = lucid::Vec2u(width, height);
        glViewport(0, 0, width, height);

        int shader_status;
        auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
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

        auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
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

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), static_cast<void*>(0));
        glEnableVertexAttribArray(0);

        // uv
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    static void
    load_img(const Image<unsigned char, 3>& img) noexcept
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.res()[0], img.res()[1], 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());
        // glGenerateMipmap(GL_TEXTURE_2D);
    }

    // static void
    // load_img(const unsigned char* data, int width, int height) noexcept
    // {
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    //     // glGenerateMipmap(GL_TEXTURE_2D);
    // }

    static bool
    active() noexcept
    { return !glfwWindowShouldClose(window); }

    static void
    draw() noexcept
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, texture);

        glUseProgram(shader_program);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    static void
    cleanup() noexcept
    {
        if(window)
            glfwDestroyWindow(window);

        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);

        glfwTerminate();
    }
}

auto
sample_hemisphere(const Normal& n, const Vec2& u) noexcept
{
    const auto& [u1, u2] = u;
    const auto r = 2_r * Pi * u2;
    const auto phi = math::sqrt(1_r - pow<2>(u1));
    return basis_matrix(n).dot(Vec3(math::cos(r) * phi,
                                    math::sin(r) * phi,
                                    u1));
}

static const constexpr std::array<Point, 8> box_points{
    Point(-1, -1, 1), Point(-1, -1, -1), Point(1, -1, -1), Point(1, -1, 1),
    Point(-1, 1, 1),  Point(-1, 1, -1),  Point(1, 1, -1),  Point(1, 1, 1)};

struct Material
{
    RGB diffuse{0, 0, 0};
    RGB emission{0, 0, 0};
};

static const constexpr Material materials[]{
    Material{RGB{0}, RGB{0}},                   // black default
    Material{RGB{1}, RGB{0}},                   // white
    Material{RGB{1, 0.1_r, 0.1_r}, RGB{0}},     // red
    Material{RGB{0.1_r, 1, 0.1_r}, RGB{0}},     // green
    Material{RGB{0.1_r, 0.5_r, 0.7_r}, RGB{0}}, // sphere
    Material{RGB{0}, RGB{1}}                    // light
};

static const constexpr std::tuple box_geo_descr{
    std::index_sequence<0, 3, 2, 1>{}, // floor
    std::index_sequence<0, 1, 5, 4>{}, // left wall
    std::index_sequence<2, 3, 7, 6>{}, // right wall
    std::index_sequence<4, 5, 6, 7>{}, // ceiling
    std::index_sequence<1, 2, 6, 5>{}  // back wall
};

static const constexpr std::array<std::size_t, 5> box_mat_idxs{
      1, // white
      2, // red
      3, // green
      1, // white
      1  // white
};

// Cornell Box Primitives
using QuadRef = Quad_<StaticSpan>;
using CBPrimTypes = typelist<Sphere, QuadRef, Disk>;
using CBPrim = typename CBPrimTypes::variant;
using ObjectData = std::pair<CBPrim, std::size_t>;

template <std::size_t... Pns>
constexpr decltype(auto)
make_wall_geo(std::index_sequence<Pns...>) noexcept
{ return QuadRef{ref(std::get<Pns>(box_points))...}; }

constexpr auto
make_room() noexcept
{
    return std::apply([](auto ... pns){ return std::tuple{make_wall_geo(pns)...}; },
                      box_geo_descr);
}

template <typename RandomEngine, typename Scene, typename MaterialGetter>
auto path_trace(Ray ray, RandomEngine& g, const Scene& scene, MaterialGetter&& mat_getter,
                const std::size_t max_depth, const real bias) noexcept
{
    RGB radiance{1};
    bool has_rad = false;

    for (std::size_t depth = 0; depth < max_depth; ++depth)
    {
        const auto& [ro, rd] = ray;
        const auto [isect, pid] = hider(ray, scene);

        if (!isect)
        {
            if (!has_rad)
                radiance = 0;
            break;
        }

        const auto& mat = mat_getter(pid);
        const auto& mat_color = mat.diffuse;
        const auto& emit_color = mat.emission;

        // const Normal i = Normal(-rd);
        const Normal n = lucid::visit(
            pid, [&, &iss = isect](const auto& prim) { return normal(ray, iss, prim); }, scene);

        const Point p = ro + rd * isect.t;
        const Normal new_dir = Normal(sample_hemisphere(n, Vec2(rand<real, 2>(g))));

        radiance *= mat_color * std::max(dot(n, new_dir), 0_r) + emit_color;
        has_rad |= any(emit_color > 0_r);

        ray = Ray(p + new_dir * bias, new_dir);
    }

    return radiance;
}

int main(/*int argc, char *argv[]*/)
{
    perspective::shoot cam(radians(60_r), look_at(Point(0, 0, 4), Point(0, 0, 0), Normal(0, 1, 0)));

    Logger logger(Logger::DEBUG);

    try
    {
        vp::init();
    }
    catch (const std::runtime_error& ex)
    {
        logger.critical("OpenGL Error: {}", ex.what());
        return 1;
    }

    logger.info("OpenGL initialized");

    const auto room_geo =
        tuple_cat(make_room(), std::tuple{Sphere(Point(0.5_r, -0.6_r, 0.2_r), 0.4_r),
                                          Disk(Point(0, 0.99_r, 0), Normal(0, -1, 0), 0.3_r)});

    const auto room_mat_ids = array_cat(box_mat_idxs, std::array<std::size_t, 2>{4, 5});

    const real bias = 0.001_r;
    std::random_device rd;
    std::default_random_engine g(rd());

    Image<unsigned char, 3> img(vp::res);

    logger.debug("Rendering image...");

    ElapsedTimer<> timer;

    for (auto it = img.begin(); it != img.end(); ++it)
    {
        const auto dc = to_device_coords(Vec2(it.pos()), Vec2(img.res()));
        const auto ray = cam(dc);

        const auto c = path_trace(
            ray, g, room_geo, [&](const std::size_t pid) { return materials[room_mat_ids[pid]]; },
            32, bias);

        const RGB8 c8 = RGB8{c * 255};
        *it = c8;
    }

    logger.info("Image is rendered in {:12%H:%M:%S}", timer.elapsed());

    vp::load_img(img);

    while (vp::active())
        vp::draw();

    vp::cleanup();

    return 0;
}
