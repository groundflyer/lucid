// -*- C++ -*-
// lucid.cpp

#include <ray_traversal/primitives/generic.hpp>
#include <cameras/perspective.hpp>
#include <cameras/utils.hpp>
#include <image/io.hpp>
#include <utils/seq.hpp>
#include <base/rng.hpp>

#include <format>

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

static const constexpr std::array<Point, 8> box_points
    {Point(-1, -1, 1),
            Point(-1, -1, -1),
            Point(1, -1, -1),
            Point(1, -1, 1),
            Point(-1, 1, 1),
            Point(-1, 1, -1),
            Point(1, 1, -1),
            Point(1, 1, 1)};

static const constexpr RGB materials[]
    {RGB{0, 0, 0},              // black default
     RGB{1, 1, 1},              // white
     RGB{1, 0.1_r, 0.1_r},      // red
     RGB{0.1_r, 1, 0.1_r},      // green
     RGB{0.1_r, 0.5_r, 0.7_r}};

static const constexpr std::tuple box_geo_descr
    {
     std::index_sequence<0, 3, 2, 1>{}, // floor
     std::index_sequence<0, 1, 5, 4>{}, // left wall
     std::index_sequence<2, 3, 7, 6>{}, // right wall
     std::index_sequence<4, 5, 6, 7>{}, // ceiling
     std::index_sequence<1, 2, 6, 5>{}  // back wall
    };

static const constexpr std::array<std::size_t, 5> box_mat_idxs
    {
     1,                         // white
     2,                         // red
     3,                         // green
     1,                         // white
     1                          // white
    };

using ObjectData = std::pair<GenericPrimitive, std::size_t>;

template <std::size_t ... Pns>
constexpr decltype(auto)
make_wall_geo(std::index_sequence<Pns...>) noexcept
{ return Quad{std::get<Pns>(box_points)...}; }

constexpr auto
make_room() noexcept
{
    return std::apply([](auto ... pns){ return std::array<GenericPrimitive, sizeof...(pns)>{make_wall_geo(pns)...}; },
                      box_geo_descr);
}


int main(/*int argc, char *argv[]*/)
{
    perspective::shoot cam(radians(60_r), look_at(Point(0, 0, 4), Point(0, 0, 0), Normal(0, 1, 0)));

    try
    {
        vp::init();
    }
    catch (const std::runtime_error& ex)
    {
        fmt::print(stderr, "OpenGL Error: {}", ex.what());
        return 1;
    }

    const auto room_geo = array_cat(make_room(),
                                    std::array<GenericPrimitive, 2>
                                    {
                                        GenericPrimitive{Sphere(Point(0.5_r, -0.6_r, 0.2_r), 0.4_r)},
                                        GenericPrimitive{Disk(Point(0, 0.99_r, 0), Normal(0, -1, 0), 0.3_r)}
                                    });

    const auto room_mat_ids = array_cat(box_mat_idxs, std::array<std::size_t, 2>{4, 1});

    const real bias = 0.001_r;
    std::random_device rd;
    std::default_random_engine g(rd());

    const constexpr std::size_t lid = 6;

    Image<unsigned char, 3> img(vp::res);

    for(auto it = img.begin(); it != img.end(); ++it)
    {
        const auto dc = to_device_coords(Vec2(it.pos()), Vec2(img.res()));
        const auto ray = cam(dc);
        const auto& [ro, rd] = ray;
        const auto [isect, pid] = hider(ray, room_geo);
        const auto& prim = room_geo[pid];
        const auto& mat_color = materials[room_mat_ids[pid]];

        // const Normal i = Normal(-rd);
        const Normal n = normal(ray, isect, prim);
        const Point p(ro + rd * isect.t);

        RGB c{};

        if (isect)
        {
            if (pid != lid)
            {
                // const Normal gidir(sample_hemisphere(n, Vec2(rand<real, 2>(g))));
                // const Ray raygi(p, gidir);
                // const auto [gi_isect, gi_hitid] = traverse_scene(raygi, room);
                // RGB gi_color{};
                // if (gi_isect)
                // {
                //     if(gi_hitid == lid)
                //         gi_color = std::max(dot(gidir, n), 0_r);
                //     else
                //     {
                //         const auto& [gi_prim, gi_mat] = room[gi_hitid];
                //         const Normal gi_n = normal(raygi, gi_isect, gi_prim);
                //         const Point gi_p = p + gidir * gi_isect.t;
                //         const Point gi_nee_p = sample(Vec2(rand<real, 2>(g)), std::get<lid>(room).first);
                //         const Normal gi_nee_dir(gi_nee_p - gi_p);
                //         const Ray gi_nee_ray(gi_p, gi_nee_dir);
                //         const auto [gi_nee_isect, gi_nee_id] = traverse_scene(gi_nee_ray, room);
                //         if(gi_nee_isect && gi_nee_id == lid)
                //             gi_color = materials[gi_mat] * std::max(dot(gi_nee_dir, gi_n), 0_r);
                        
                //     }
                // }
                const Point lp = sample(Vec2(rand<real, 2>(g)), std::get<lid>(room_geo));
                const Normal shray_dir = Normal(lp - p);
                const Ray shadow_ray(p + shray_dir * bias, shray_dir);
                const auto [si, spid] = hider(shadow_ray, room_geo);
                if (si && spid == lid)
                    c = mat_color * std::max(dot(shray_dir, n), 0_r) / (1 + pow<2>(si.t));
            }
            else
                c = RGB{1};
        }
        // const auto dd = dot(i, n);
        // const RGB c(materials[pm.second] * dd);
        // const RGB c(isect.t * 0.1);
        const RGB8 c8 = RGB8{c * 255};
        *it = c8;
    }

    vp::load_img(img);

    while(vp::active())
        vp::draw();

    vp::cleanup();

    return 0;
}
