struct vector2d {
    int x, y;
};

struct vec3 {
    float x, y, z;
};

struct vector3d_n {
    float x, y, z;
};

struct argb_color {
    uint8_t a, r, g, b;
};

struct vertex {
    vector3d_n position;
    argb_color color;
    float u, v;
};

struct fragment_shader_data {
    vertex v1;
    vertex v2;
    vertex v3;
};

bool debug_mode = true;

uint32_t* framebuffer = nullptr;
vector2d win_size = {800, 600};


inline vector2d normalize_to_screen(vector3d_n p, vector2d win_size) {
    return {
        static_cast<int>((p.x + 1.0f) * 0.5f * win_size.x),
        static_cast<int>((1.0f - p.y) * 0.5f * win_size.y)
    };
}


inline void set_pixel(uint32_t* framebuffer, vector2d win_size, vector2d pixel, uint32_t color) {
    if (pixel.x < 0 || pixel.x >= win_size.x || pixel.y < 0 || pixel.y >= win_size.y) return;
    framebuffer[pixel.y * win_size.x + pixel.x] = color;
}


inline void fill_rect(uint32_t* framebuffer, vector2d win_size, uint32_t color) {
    std::fill(framebuffer, framebuffer + win_size.x * win_size.y, color);
}


inline void draw_line(uint32_t* framebuffer, vector2d win_size, vector3d_n p0, vector3d_n p1, uint32_t color) {
    // Fokin Bresenham's algoritmo

    vector2d a = normalize_to_screen(p0, win_size);
    vector2d b = normalize_to_screen(p1, win_size);

    int dx = std::abs(b.x - a.x);
    int dy = std::abs(b.y - a.y);
    int sx = (a.x < b.x) ? 1 : -1;
    int sy = (a.y < b.y) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        set_pixel(framebuffer, win_size, a, color);

        if (a.x == b.x && a.y == b.y) break;

        int e2 = err * 2;

        if (e2 > -dy) {
            err -= dy;
            a.x += sx;
        }

        if (e2 < dx) {
            err += dx;
            a.y += sy;
        }
    }
}


inline float edge_function(vector2d a, vector2d b, vector2d p) {
    return static_cast<float>(p.x - a.x) * (b.y - a.y) -
           static_cast<float>(p.y - a.y) * (b.x - a.x);
}


inline void debug_draw_line(bool debug) {
    debug_mode = debug;
}


inline fragment_shader_data init_fragment_shader(vertex v1, vertex v2, vertex v3, vec3 barycentric) {
    v1.color.r = static_cast<uint8_t>(v1.color.r * barycentric.x);
    v1.color.g = static_cast<uint8_t>(v1.color.g * barycentric.x);
    v1.color.b = static_cast<uint8_t>(v1.color.b * barycentric.x);

    v2.color.r = static_cast<uint8_t>(v2.color.r * barycentric.y);
    v2.color.g = static_cast<uint8_t>(v2.color.g * barycentric.y);
    v2.color.b = static_cast<uint8_t>(v2.color.b * barycentric.y);

    v3.color.r = static_cast<uint8_t>(v3.color.r * barycentric.z);
    v3.color.g = static_cast<uint8_t>(v3.color.g * barycentric.z);
    v3.color.b = static_cast<uint8_t>(v3.color.b * barycentric.z);

    return {v1, v2, v3};
}

inline argb_color fragment_shader(const fragment_shader_data& data) {
    return {
        255,
        static_cast<uint8_t>(data.v1.color.r + data.v2.color.r + data.v3.color.r),
        static_cast<uint8_t>(data.v1.color.g + data.v2.color.g + data.v3.color.g),
        static_cast<uint8_t>(data.v1.color.b + data.v2.color.b + data.v3.color.b)
    };
}


inline uint32_t argb_color_to_uint32(argb_color color) {
    return (static_cast<uint32_t>(color.a) << 24) |
           (static_cast<uint32_t>(color.r) << 16) |
           (static_cast<uint32_t>(color.g) << 8) |
           static_cast<uint32_t>(color.b);
}


void draw_triangle(uint32_t* framebuffer, vector2d win_size, const vertex& p0, const vertex& p1, const vertex& p2, uint32_t color) {
    if (debug_mode) {
        draw_line(framebuffer, win_size, p0.position, p1.position, color);
        draw_line(framebuffer, win_size, p1.position, p2.position, color);
        draw_line(framebuffer, win_size, p2.position, p0.position, color);
        return;
    }

    vector2d a = normalize_to_screen(p0.position, win_size);
    vector2d b = normalize_to_screen(p1.position, win_size);
    vector2d c = normalize_to_screen(p2.position, win_size);

    float area = edge_function(a, b, c);
    if (area == 0.0f) return;

    int min_x = std::max(0, std::min({a.x, b.x, c.x}));
    int max_x = std::min(win_size.x - 1, std::max({a.x, b.x, c.x}));
    int min_y = std::max(0, std::min({a.y, b.y, c.y}));
    int max_y = std::min(win_size.y - 1, std::max({a.y, b.y, c.y}));

    float inverse_area = 1.0f / area;

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            vector2d p = {x, y};

            float w0 = edge_function(b, c, p) * inverse_area;
            float w1 = edge_function(c, a, p) * inverse_area;
            float w2 = edge_function(a, b, p) * inverse_area;
            
            //si no esta en el triangulo, lo ignora
            if (!((w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) ||
                  (w0 <= 0.0f && w1 <= 0.0f && w2 <= 0.0f))) {
                continue;
            }

            vec3 barycentric = {w0, w1, w2};
            fragment_shader_data shader_data = init_fragment_shader(p0, p1, p2, barycentric);
            argb_color fragment_color = fragment_shader(shader_data);

            framebuffer[y * win_size.x + x] = argb_color_to_uint32(fragment_color);
        }
    }
}


void render_loop(uint32_t* framebuffer, vector2d win_size) {
    fill_rect(framebuffer, win_size, 0xFF000000);
    debug_draw_line(false);
    vertex v1 = {{-0.5f, -0.5f, 0.0f}, {255, 255, 0, 0}, 0.0f, 0.0f};
    vertex v2 = {{0.0f, 0.5f, 0.0f}, {255, 0, 255, 0}, 0.5f, 0.5f};
    vertex v3 = {{0.5f, -0.5f, 0.0f}, {255, 0, 0, 255}, 1.0f, 1.0f};
    draw_triangle(framebuffer,win_size,
        v1,v2,v3,
        0xFF00FF00
    );
}
