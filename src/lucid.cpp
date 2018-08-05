// -*- C++ -*-
// lucid.cpp

#include <scene/geometry_object.hpp>
#include <cameras/perspective.hpp>
#include <io/image.hpp>

#include <GLFW/glfw3.h>

#include <string>
#include <stdexcept>


using namespace yapt;

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

    static const constexpr float vertex_data[] = {
        // positions    // texture coordinates
        0.9f,  -0.9f, 0,       1, 1,
        -0.9f, -0.9f, 0,       1, 0,
        -0.9f, 0.9f, 0,      0, 0,
        0.9f,  0.9f, 0,      0, 1
    };

    static const constexpr unsigned indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    static GLFWwindow* window;

    static unsigned shader_program;
    static unsigned VBO;
    static unsigned VAO;
    static unsigned EBO;
    static unsigned texture;

    static Vec2u res;

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

        window = glfwCreateWindow(640, 480, "Lucid", nullptr, nullptr);
        res = Vec2u(640, 480);

        if(!window)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create window");
        }

        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, resize);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        res = Vec2u(width, height);
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
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
        glGenerateMipmap(GL_TEXTURE_2D);
    }

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

int main(int argc, char *argv[])
{
    // size_t resx = 640;
    // size_t resy = 480;
    // if(argc == 3)
    // {
    //     resx = std::stoul(argv[1]);
    //     resy = std::stoul(argv[2]);
    // }
    // const Vec2u res(resx, resy);
    // std::cout << "Res: " << resx << ' ' << resy << std::endl;

    const Point sphere_pos(0,1,0);
    const Point disk_pos(-1,1,1);
    const Sphere sphere(sphere_pos, 1);
    const Plane plane(Point(0, 0, 0), Normal(0,1,0));
    std::vector<GenericPrimitive> prims;
    prims.push_back(sphere);
    prims.push_back(plane);
    prims.push_back(Disk(disk_pos, Normal(sphere_pos - disk_pos), 1_r));
    prims.push_back(AABB(Point(-6,0,1), Point(-3,4,3)));

    const Point lp(-2, 5, -3);

    PerspectiveCamera cam(math::radians(120_r),
                          look_at(Point(0,4,-6), sphere_pos));

    try
    {
        vp::init();
    }
    catch (const std::runtime_error& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    
    Image<unsigned char, 3> img(vp::res);

    for(auto it = img.begin(); it != img.end(); ++it)
    {
        const auto dc = to_device_coords(it.pos(), vp::res);
        const auto ray = cam(dc);
        const auto isect = traverse(ray, prims);
        const auto p = get_intersection_pos(ray, isect.first);
        const auto i = -ray.dir;
        const auto n = compute_normal(ray, isect.first, *isect.second);
        const auto L = lp - p;
        const auto Ld = length(L);
        const Normal l(L / Ld);
        const auto shadow = occlusion(Ray(p, l), prims, Range<real>(std::numeric_limits<real>::min() + std::numeric_limits<real>::epsilon() * 1000, Ld));
        const RGB c(std::max(n.dot(l), 0_r) / (math::pow<3>(Ld)) * 30 * !shadow);
        const Vector<unsigned char, 3, std::array> c8(yapt::min(c * 255, RGB(255)));
        *it = c8;
        // *it = fit(n, min(n), max(n));
    }

    vp::load_img(img);

    while(vp::active())
        vp::draw();

    vp::cleanup();

    write_ppm(img, "render.ppm");

    return 0;
}
