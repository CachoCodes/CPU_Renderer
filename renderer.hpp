struct Vector2D {
    int x, y;
};
struct Vector3D_N {
    float x, y, z;
};

bool DebugMode = true;

uint32_t* framebuffer = nullptr;

Vector2D winSize = {800, 600};

Vector2D normalizeToScreen(Vector3D_N p, Vector2D winSize) {
    return {
        static_cast<int>((p.x + 1.0f) * 0.5f * winSize.x),
        static_cast<int>((1.0f - p.y) * 0.5f * winSize.y)
    };
}

void setPixel(uint32_t* framebuffer, Vector2D winSize, Vector2D pixel, uint32_t color) {
    if (pixel.x < 0 || pixel.x >= winSize.x || pixel.y < 0 || pixel.y >= winSize.y) return;
    framebuffer[pixel.y * winSize.x + pixel.x] = color;
}

void FillRect(uint32_t* framebuffer, Vector2D winSize, uint32_t color) {
    for (int y = 0; y < winSize.y; y++)
        for (int x = 0; x < winSize.x; x++)
            framebuffer[y * winSize.x + x] = color;
}

void drawLine(uint32_t* framebuffer, Vector2D winSize, Vector3D_N p0, Vector3D_N p1, uint32_t color) {
    Vector2D a = normalizeToScreen(p0, winSize);
    Vector2D b = normalizeToScreen(p1, winSize);

    int dx = abs(b.x - a.x);
    int dy = abs(b.y - a.y);
    int sx = (a.x < b.x) ? 1 : -1;
    int sy = (a.y < b.y) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        setPixel(framebuffer, winSize, a, color);

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

void DebugDrawLine(bool debug) {
    DebugMode = debug;
}

void drawTriangle(uint32_t* framebuffer, Vector2D winSize, Vector3D_N p0, Vector3D_N p1, Vector3D_N p2, uint32_t color) {
    if (DebugMode) {
        drawLine(framebuffer, winSize, p0, p1, color);
        drawLine(framebuffer, winSize, p1, p2, color);
        drawLine(framebuffer, winSize, p2, p0, color);
    }
    else {
        //Aplicar formula q esta escrita en la hoja lol
    }
}

void RENDERLOOP(uint32_t* framebuffer, Vector2D winSize) {
    FillRect(framebuffer, winSize, 0xFF000000);
    DebugDrawLine(true);
    drawTriangle(framebuffer, winSize, Vector3D_N{-0.5f, -0.5f, 0.0f}, Vector3D_N{0.0f, 0.5f, 0.0f}, Vector3D_N{0.5f, -0.5f, 0.0f}, 0xFF00FF00);
}