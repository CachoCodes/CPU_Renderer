struct Vector2D {
    //Vector 2D para coordenadas de la ventana
    int x, y;
};
struct Vector3D_N {
    //Vector normalizado
    float x, y, z;
};
struct ARGB_Color {
    //Color en formato ARGB
    uint8_t a, r, g, b;
};
struct Vertex {
    //bld es un vertice no te sobre compliques
    Vector3D_N position;
    float u, v;

};

bool DebugMode = true;
//Creo q es auto explicativo

uint32_t* framebuffer = nullptr;
//El framebuffer es un array de uint32_t q representa los pixeles de la ventana (32bits pq es RGBA con colores de 8bits)

Vector2D winSize = {800, 600};
//Tamaño de la ventana

inline Vector2D normalizeToScreen(Vector3D_N p, Vector2D winSize) {
    //Una helper para normalizar las coordenadas de un punto 3D a coordenadas de la ventana
    return {
        static_cast<int>((p.x + 1.0f) * 0.5f * winSize.x),
        static_cast<int>((1.0f - p.y) * 0.5f * winSize.y)
    };
}

void setPixel(uint32_t* framebuffer, Vector2D winSize, Vector2D pixel, uint32_t color) {
    //Poner un pixel en la pantalla con un color especifico
    if (pixel.x < 0 || pixel.x >= winSize.x || pixel.y < 0 || pixel.y >= winSize.y) return;
    framebuffer[pixel.y * winSize.x + pixel.x] = color;
}

void FillRect(uint32_t* framebuffer, Vector2D winSize, uint32_t color) {
    //Llenar un rectangulo en la pantalla con un color especifico
    for (int y = 0; y < winSize.y; y++)
        for (int x = 0; x < winSize.x; x++)
            framebuffer[y * winSize.x + x] = color;
}

void drawLine(uint32_t* framebuffer, Vector2D winSize, Vector3D_N p0, Vector3D_N p1, uint32_t color) {
    //Dibujar una linea entre dos puntos usando el algoritmo de Bresenham

    //Convertir a coordenadas de la window
    Vector2D a = normalizeToScreen(p0, winSize);
    Vector2D b = normalizeToScreen(p1, winSize);
    
    //Fokin Bresenham's algoritmo
    int dx = abs(b.x - a.x);
    int dy = abs(b.y - a.y);
    int sx = (a.x < b.x) ? 1 : -1;
    int sy = (a.y < b.y) ? 1 : -1;
    int err = dx - dy;

    //Loop hasta q dibujemos la linea
    //Tengo q mejorar los nombres de mis variables q despues no entiendo un pingo
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

float edgeFunction(Vector2D a, Vector2D b, Vector2D p) {
    //Calcula el area del triangulo mediante un CrossProduct en 2D
    return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
}

inline void DebugDrawLine(bool debug) {
    //Helper para activar o desactivar el modo debug
    DebugMode = debug;
}

ARGB_Color vertexShaderCode(float u, float v) {
    //Un shader lol
    ARGB_Color color;
    color.r = static_cast<uint8_t>(u * 255);
    color.g = static_cast<uint8_t>(v * 255);
    color.b = 128;
    color.a = 255;
    return color;
}

uint32_t ARGB_Color_to_uint32(ARGB_Color color) {
    uint32_t result = (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b; 
    return result;
}


void drawTriangle(uint32_t* framebuffer, Vector2D winSize, Vertex p0, Vertex p1, Vertex p2, uint32_t color) {
    if (DebugMode) {
        drawLine(framebuffer, winSize, p0.position, p1.position, color);
        drawLine(framebuffer, winSize, p1.position, p2.position, color);
        drawLine(framebuffer, winSize, p2.position, p0.position, color);
    }

    else {
        //Convertir a coordenadas de la window
        Vector2D a = normalizeToScreen(p0.position, winSize);
        Vector2D b = normalizeToScreen(p1.position, winSize);
        Vector2D c = normalizeToScreen(p2.position, winSize);
        //Sacar los maximos para evitar loopear por toda la fokin window
        int minX = std::min({a.x, b.x, c.x});
        int maxX = std::max({a.x, b.x, c.x});
        int minY = std::min({a.y, b.y, c.y});
        int maxY = std::max({a.y, b.y, c.y});

        //Calcular el area del triangulo para usar en el coso baricentrico
        float area = edgeFunction(a, b, c);
        
        //Rasterizado 
        for (int y = minY; y <= maxY; y++) {
            for (int x = minX; x <= maxX; x++) {
                //El punto q se usa para calcular los baricentros
                Vector2D p = {x, y};

                //Calculadermis y Normalizadermis          
                float w0 = edgeFunction(b, c, p)/area;
                float w1 = edgeFunction(c, a, p)/area;
                float w2 = edgeFunction(a, b, p)/area;
                

                if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)){
                    //Calculamos las coordenadas de las texturas
                    float u = w0 * p0.u + w1 * p1.u + w2 * p2.u;
                    float v = w0 * p0.v + w1 * p1.v + w2 * p2.v;
                    //Le ponemos el shader
                    setPixel(framebuffer, winSize, p, ARGB_Color_to_uint32(ARGB_Color{255,static_cast<uint8_t>(w0 * 255),static_cast<uint8_t>(w1 * 255),static_cast<uint8_t>(w2 * 255)}));
                }
        }
    }
}
}

void RENDERLOOP(uint32_t* framebuffer, Vector2D winSize) {
    FillRect(framebuffer, winSize, 0xFF000000);
    DebugDrawLine(false);
    drawTriangle(framebuffer, winSize, Vertex{{-0.5f, -0.5f, 0.0f}, 0.0f, 0.0f}, Vertex{{0.0f, 0.5f, 0.0f}, 0.5f, 0.5f}, Vertex{{0.5f, -0.5f, 0.0f}, 1.0f, 1.0f}, 0xFF00FF00);
}