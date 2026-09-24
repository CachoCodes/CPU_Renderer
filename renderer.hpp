#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
 
struct mat4
{
    float m[4][4];

    static mat4 zero()
    {
        return mat4{{{0.0f, 0.0f, 0.0f, 0.0f},
                     {0.0f, 0.0f, 0.0f, 0.0f},
                     {0.0f, 0.0f, 0.0f, 0.0f},
                     {0.0f, 0.0f, 0.0f, 0.0f}}};
    }

    static mat4 identity()
    {
        return mat4{{{1.0f, 0.0f, 0.0f, 0.0f},
                     {0.0f, 1.0f, 0.0f, 0.0f},
                     {0.0f, 0.0f, 1.0f, 0.0f},
                     {0.0f, 0.0f, 0.0f, 1.0f}}};
    }

    mat4 operator*(const mat4 &other) const
    {
        mat4 result = mat4::zero();

        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                for (int k = 0; k < 4; ++k)
                {
                    result.m[row][col] += m[row][k] * other.m[k][col];
                }
            }
        }

        return result;
    }

    static mat4 translate(float x, float y, float z)
    {
        mat4 matrix = mat4::identity();

        matrix.m[3][0] = x;
        matrix.m[3][1] = y;
        matrix.m[3][2] = z;

        return matrix;
    }

    static mat4 scale(float sx, float sy, float sz)
    {
        mat4 matrix = mat4::identity();

        matrix.m[0][0] = sx;
        matrix.m[1][1] = sy;
        matrix.m[2][2] = sz;

        return matrix;
    }

    static mat4 rotate_x(float degrees)
    {
        float radians = degrees * (PI / 180.0f);
        float c = std::cos(radians);
        float s = std::sin(radians);

        mat4 matrix = mat4::identity();

        matrix.m[1][1] = c;
        matrix.m[1][2] = -s;
        matrix.m[2][1] = s;
        matrix.m[2][2] = c;

        return matrix;
    }

    static mat4 rotate_y(float degrees)
    {
        float radians = degrees * (PI / 180.0f);
        float c = std::cos(radians);
        float s = std::sin(radians);

        mat4 matrix = mat4::identity();

        matrix.m[0][0] = c;
        matrix.m[0][2] = s;
        matrix.m[2][0] = -s;
        matrix.m[2][2] = c;

        return matrix;
    }

    static mat4 rotate_z(float degrees)
    {
        float radians = degrees * (PI / 180.0f);
        float c = std::cos(radians);
        float s = std::sin(radians);

        mat4 matrix = mat4::identity();

        matrix.m[0][0] = c;
        matrix.m[0][1] = -s;
        matrix.m[1][0] = s;
        matrix.m[1][1] = c;

        return matrix;
    }

    static mat4 perspective(float fov_degrees, float aspect, float near_plane, float far_plane)
    {
        mat4 matrix = mat4::zero();

        float fov_radians = fov_degrees * (PI / 180.0f);
        float tan_half_fov = std::tan(fov_radians / 2.0f);

        matrix.m[0][0] = 1.0f / (aspect * tan_half_fov);
        matrix.m[1][1] = 1.0f / tan_half_fov;

        matrix.m[2][2] = far_plane / (far_plane - near_plane);
        matrix.m[2][3] = 1.0f;
        matrix.m[3][2] = (-far_plane * near_plane) / (far_plane - near_plane);

        return matrix;
    }
};

struct vec2i
{
    int x, y;
};

struct vec2f
{
    float x, y;
};

struct vec3f
{
    float x, y, z;

    vec3f operator+(const vec3f &o) const
    {
        return {x + o.x, y + o.y, z + o.z};
    }

    vec3f operator-(const vec3f &o) const
    {
        return {x - o.x, y - o.y, z - o.z};
    }

    vec3f operator*(float scalar) const
    {
        return {x * scalar, y * scalar, z * scalar};
    }

    float dot(const vec3f &o) const
    {
        return x * o.x + y * o.y + z * o.z;
    }

    vec3f cross(const vec3f &o) const
    {
        return {
            y * o.z - z * o.y,
            z * o.x - x * o.z,
            x * o.y - y * o.x};
    }

    vec3f transform(const mat4 &mat) const
    {
        float nx = x * mat.m[0][0] +
                   y * mat.m[1][0] +
                   z * mat.m[2][0] +
                   mat.m[3][0];

        float ny = x * mat.m[0][1] +
                   y * mat.m[1][1] +
                   z * mat.m[2][1] +
                   mat.m[3][1];

        float nz = x * mat.m[0][2] +
                   y * mat.m[1][2] +
                   z * mat.m[2][2] +
                   mat.m[3][2];

        float nw = x * mat.m[0][3] +
                   y * mat.m[1][3] +
                   z * mat.m[2][3] +
                   mat.m[3][3];

        if (nw != 1.0f && nw != 0.0f)
        {
            nx /= nw;
            ny /= nw;
            nz /= nw;
        }

        return {nx, ny, nz};
    }
};

struct vec3i
{
    int x, y, z;
};

struct argb_color
{
    uint8_t a, r, g, b;
};

struct vertex
{
    vec3f position;
    argb_color color;
    float u, v;
};

struct fragment_shader_data
{
    float u;
    float v;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct task {
        float priority;
        std::function<void()> function;
};


bool debug_mode = true;

uint32_t *framebuffer = nullptr;
float *depth_buffer = nullptr;
vec2i win_size = {800, 600};

inline vec2i normalize_to_screen(vec3f p, vec2i win_size)
{
    return {
        static_cast<int>((p.x + 1.0f) * 0.5f * win_size.x),
        static_cast<int>((1.0f - p.y) * 0.5f * win_size.y)};
}

inline void set_pixel(uint32_t *framebuffer, vec2i win_size, vec2i pixel, uint32_t color)
{
    if (pixel.x < 0 || pixel.x >= win_size.x || pixel.y < 0 || pixel.y >= win_size.y)
        return;
    framebuffer[pixel.y * win_size.x + pixel.x] = color;
}

inline void fill_rect(uint32_t *framebuffer, vec2i win_size, uint32_t color)
{
    std::fill(framebuffer, framebuffer + win_size.x * win_size.y, color);
}

inline void draw_line(uint32_t *framebuffer, vec2i win_size, vec3f p0, vec3f p1, uint32_t color)
{
    // Fokin Bresenham's algoritm

    vec2i a = normalize_to_screen(p0, win_size);
    vec2i b = normalize_to_screen(p1, win_size);

    int dx = std::abs(b.x - a.x);
    int dy = std::abs(b.y - a.y);
    int sx = (a.x < b.x) ? 1 : -1;
    int sy = (a.y < b.y) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        set_pixel(framebuffer, win_size, a, color);

        if (a.x == b.x && a.y == b.y)
            break;

        int e2 = err * 2;

        if (e2 > -dy)
        {
            err -= dy;
            a.x += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            a.y += sy;
        }
    }
}

inline float edge_function(vec2i a, vec2i b, vec2i p)
{
    return static_cast<float>(p.x - a.x) * (b.y - a.y) -
           static_cast<float>(p.y - a.y) * (b.x - a.x);
}

inline void debug_draw_line(bool debug)
{
    debug_mode = debug;
}

inline fragment_shader_data init_fragment_shader(vertex v1, vertex v2, vertex v3, vec3f barycentric)
{
    // Interpolacion de valores
    v1.color.r = static_cast<uint8_t>(v1.color.r * barycentric.x);
    v1.color.g = static_cast<uint8_t>(v1.color.g * barycentric.x);
    v1.color.b = static_cast<uint8_t>(v1.color.b * barycentric.x);
    v1.u = (v1.u * barycentric.x);
    v1.v = (v1.v * barycentric.x);

    v2.color.r = static_cast<uint8_t>(v2.color.r * barycentric.y);
    v2.color.g = static_cast<uint8_t>(v2.color.g * barycentric.y);
    v2.color.b = static_cast<uint8_t>(v2.color.b * barycentric.y);
    v2.u = (v2.u * barycentric.y);
    v2.v = (v2.v * barycentric.y);

    v3.color.r = static_cast<uint8_t>(v3.color.r * barycentric.z);
    v3.color.g = static_cast<uint8_t>(v3.color.g * barycentric.z);
    v3.color.b = static_cast<uint8_t>(v3.color.b * barycentric.z);
    v3.u = (v3.u * barycentric.z);
    v3.v = (v3.v * barycentric.z);

    float u = v1.u + v2.u + v3.u;
    float v = v1.v + v2.v + v3.v;
    uint8_t r = v3.color.r + v2.color.r + v1.color.r;
    uint8_t g = v3.color.g + v2.color.g + v1.color.g;
    uint8_t b = v3.color.b + v2.color.b + v1.color.b;

    return fragment_shader_data{u, v, r, g, b};
}

inline argb_color texture_mapping(vec2f pos, unsigned char *image, int width, int height)
{
    argb_color frag_color;
    int x = static_cast<int>(pos.x * width);
    int y = static_cast<int>(pos.y * height);

    x = std::max(0, std::min(x, width - 1));
    y = std::max(0, std::min(y, height - 1));
    int index = (y * width + x) * 3;

    frag_color.a = 255;
    frag_color.r = image[index + 0];
    frag_color.g = image[index + 1];
    frag_color.b = image[index + 2];
    return frag_color;
}

int width, height, channels;
unsigned char *image;
inline argb_color fragment_shader(const fragment_shader_data &data)
{
    // EL COLOR ESTA INTERPOLADO PERO NO NORMALIZADO, ESTA EN UINT8_T
    argb_color frag_color;

    frag_color = texture_mapping({data.u, data.v}, image, width, height);
    // frag_color.r = (frag_color.r * data.r) / 255;
    // frag_color.g = (frag_color.g * data.g) / 255;
    // frag_color.b = (frag_color.b * data.b) / 255;

    // frag_color = {255, data.r,data.g,data.b};
    return frag_color;
}

inline uint32_t argb_color_to_uint32(argb_color color)
{
    return (static_cast<uint32_t>(color.a) << 24) |
           (static_cast<uint32_t>(color.r) << 16) |
           (static_cast<uint32_t>(color.g) << 8) |
           static_cast<uint32_t>(color.b);
}



void draw_triangle(uint32_t *framebuffer, vec2i win_size, const vertex &p0, const vertex &p1, const vertex &p2, uint32_t color)
{
    if (debug_mode)
    {
        draw_line(framebuffer, win_size, p0.position, p1.position, color);
        draw_line(framebuffer, win_size, p1.position, p2.position, color);
        draw_line(framebuffer, win_size, p2.position, p0.position, color);
        return;
    }

    vec2i a = normalize_to_screen(p0.position, win_size);
    vec2i b = normalize_to_screen(p1.position, win_size);
    vec2i c = normalize_to_screen(p2.position, win_size);

    float area = edge_function(a, b, c);
    if (area == 0.0f)
        return;

    int min_x = std::max(0, std::min({a.x, b.x, c.x}));
    int max_x = std::min(win_size.x - 1, std::max({a.x, b.x, c.x}));
    int min_y = std::max(0, std::min({a.y, b.y, c.y}));
    int max_y = std::min(win_size.y - 1, std::max({a.y, b.y, c.y}));

    float inverse_area = 1.0f / area;

    for (int y = min_y; y <= max_y; y++)
    {
        for (int x = min_x; x <= max_x; x++)
        {
            vec2i p = {x, y};

            float w0 = edge_function(b, c, p) * inverse_area;
            float w1 = edge_function(c, a, p) * inverse_area;
            float w2 = edge_function(a, b, p) * inverse_area;

            // si no esta en el triangulo ignora
            if (!((w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) ||
                  (w0 <= 0.0f && w1 <= 0.0f && w2 <= 0.0f)))
            {
                continue;
            }

            // --- test de profundidad ---
            float depth = w0 * p0.position.z + w1 * p1.position.z + w2 * p2.position.z;

            int idx = y * win_size.x + x;
            if (depth >= depth_buffer[idx])
                continue; // ya hay algo más cerca dibujado en este pixel

            depth_buffer[idx] = depth;

            vec3f barycentric = {w0, w1, w2};
            fragment_shader_data shader_data = init_fragment_shader(p0, p1, p2, barycentric);
            argb_color fragment_color = fragment_shader(shader_data);

            framebuffer[idx] = argb_color_to_uint32(fragment_color);
        }
    }
}


void visualize_depth_buffer(uint32_t *framebuffer, vec2i win_size)
{
    int count = win_size.x * win_size.y;

    float min_depth = 1e9f;
    float max_depth = -1e9f;

    for (int i = 0; i < count; i++)
    {
        float d = depth_buffer[i];
        if (d >= 1e9f) continue; 
        min_depth = std::min(min_depth, d);
        max_depth = std::max(max_depth, d);
    }

    float range = max_depth - min_depth;
    if (range <= 0.0f) range = 1.0f;

    for (int i = 0; i < count; i++)
    {
        float d = depth_buffer[i];

        uint8_t gray;
        if (d >= 1e9f)
        {
            gray = 0; 
        }
        else
        {
            float t = (d - min_depth) / range; 
            gray = static_cast<uint8_t>((1.0f - t) * 255.0f); 
        }

        argb_color c = {255, gray, gray, gray};
        framebuffer[i] = argb_color_to_uint32(c);
    }
}

void init_render()
{
    debug_draw_line(false);

    stbi_set_flip_vertically_on_load(true);
    image = stbi_load("image.png", &width, &height, &channels, 3);

    if (!image)
    {
        std::cout << stbi_failure_reason() << "\n";
        throw std::runtime_error("shit");
    }

    char buffer[128];
    std::snprintf(buffer, sizeof(buffer), "OK: %dx%d channels=%d\n", width, height, channels);
    OutputDebugStringA(buffer);
}

inline void clear_z_buffer(){
    std::fill(depth_buffer, depth_buffer + win_size.x * win_size.y, 1e9f);
}

void render_loop(uint32_t *framebuffer, vec2i win_size)
{
    clear_z_buffer();
    fill_rect(framebuffer, win_size, 0x00000000);

    vertex t_apex_front = {{0.0f, 0.5f, 0.0f},{255, 255, 255, 255},0.5f,0.0f};
    vertex t_b1_front = {{-0.5f, -0.5f, -0.5f},{255, 255, 255, 255},0.0f,1.0f};
    vertex t_b2_front = {{0.5f, -0.5f, -0.5f},{255, 255, 255, 255},1.0f,1.0f};
    vertex t_apex_right = {{0.0f, 0.5f, 0.0f},{255, 255, 255, 255},0.5f,0.0f};
    vertex t_b2_right = {{0.5f, -0.5f, -0.5f},{255, 255, 255, 255},0.0f,1.0f};
    vertex t_b3_right = {{0.5f, -0.5f, 0.5f},{255, 255, 255, 255},1.0f,1.0f};
    vertex t_apex_back = {{0.0f, 0.5f, 0.0f},{255, 255, 255, 255},0.5f,0.0f};
    vertex t_b3_back = {{0.5f, -0.5f, 0.5f},{255, 255, 255, 255},0.0f,1.0f};
    vertex t_b4_back = {{-0.5f, -0.5f, 0.5f},{255, 255, 255, 255},1.0f,1.0f};
    vertex t_apex_left = {{0.0f, 0.5f, 0.0f},{255, 255, 255, 255},0.5f,0.0f};
    vertex t_b4_left = {{-0.5f, -0.5f, 0.5f},{255, 255, 255, 255},0.0f,1.0f};
    vertex t_b1_left = {{-0.5f, -0.5f, -0.5f},{255, 255, 255, 255},1.0f,1.0f};
    vertex t_b1_base = {{-0.5f, -0.5f, -0.5f},{255, 255, 255, 255},0.0f,0.0f};
    vertex t_b2_base = {{0.5f, -0.5f, -0.5f},{255, 255, 255, 255},1.0f,0.0f};
    vertex t_b3_base = {{0.5f, -0.5f, 0.5f},{255, 255, 255, 255},1.0f,1.0f};
    vertex t_b4_base = {{-0.5f, -0.5f, 0.5f}, {255, 255, 255, 255}, 0.0f, 1.0f};

    static float angle = 0.0f;
    angle += 60.0f * delta;

    if (angle >= 360.0f) angle -= 360.0f;

    mat4 model = mat4::rotate_y(angle) * mat4::rotate_x(angle);
    mat4 view = mat4::translate(0.0f,0.0f,2.0f);
    float aspect = static_cast<float>(win_size.x) /static_cast<float>(win_size.y);
    mat4 proj = mat4::perspective(60.0f,aspect,0.1f,100.0f);
    mat4 mvp = model * view * proj;

    auto xform = [&](vertex v) -> vertex{
        v.position = v.position.transform(mvp);
        return v;
    };
    // Frente
    t_apex_front = xform(t_apex_front);
    t_b1_front = xform(t_b1_front);
    t_b2_front = xform(t_b2_front);
    // Derecha
    t_apex_right = xform(t_apex_right);
    t_b2_right = xform(t_b2_right);
    t_b3_right = xform(t_b3_right);
    // Atrás
    t_apex_back = xform(t_apex_back);
    t_b3_back = xform(t_b3_back);
    t_b4_back = xform(t_b4_back);
    // Izquierda
    t_apex_left = xform(t_apex_left);
    t_b4_left = xform(t_b4_left);
    t_b1_left = xform(t_b1_left);
    // Base
    t_b1_base = xform(t_b1_base);
    t_b2_base = xform(t_b2_base);
    t_b3_base = xform(t_b3_base);
    t_b4_base = xform(t_b4_base);

    draw_triangle(framebuffer,win_size,t_apex_front,t_b1_front,t_b2_front,0xFF0000FF);
    draw_triangle(framebuffer,win_size,t_apex_right,t_b2_right,t_b3_right,0xFF00FF00);
    draw_triangle(framebuffer,win_size,t_apex_back,t_b3_back,t_b4_back,0xFFFF0000);
    draw_triangle(framebuffer,win_size,t_apex_left,t_b4_left,t_b1_left,0xFFFFFF00);
    draw_triangle(framebuffer,win_size,t_b1_base,t_b2_base,t_b3_base,0xFF888888);
    draw_triangle(framebuffer,win_size,t_b1_base,t_b3_base,t_b4_base,0xFF888888);
}
