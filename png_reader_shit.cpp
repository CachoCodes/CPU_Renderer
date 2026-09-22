#include <vector>
#include <fstream>
#include <cstdint>
#include <iostream>
#include <exception>
#include <stdexcept>
/*
muy denso lo q voy a hacer, no soy masoquista btw
SIN IA fack los vibe-coders 
solo yo y la forkin pc...
Horas gastadas:
*/
struct image_file {
    uint32_t width;
    uint32_t height;
    std::vector<uint8_t> a_channel;
    std::vector<uint8_t> r_channel;
    std::vector<uint8_t> g_channel;
    std::vector<uint8_t> b_channel;
    //ARGB shit
};



image_file read_image(const std::string& filename) {
    image_file result;
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("no se pudo abrir: " + filename);
    }
    else{
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<uint8_t> png_data(size);
        std::vector<uint8_t> png_signature{0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

        if (!file.read(reinterpret_cast<char*>(png_data.data()), size)) {
            throw std::runtime_error("Error leyendo los datos de: " + filename);
        }

        if (png_data.size() < 8) {
            throw std::runtime_error("png demasiado corto: " + filename);
        }

        if (!std::equal(png_signature.begin(), png_signature.end(), png_data.begin())) {
            throw std::runtime_error("png no valido: " + filename);
        }
        
        //parsear los datos del png
        //leer el chunk
        
        //leer el chunk length y el chunk type
        uint8_t chunk_length_bytes[4];
        std::copy(png_data.data() + 8, png_data.data() + 12, chunk_length_bytes);

        char chunk_type_bytes[4];
        std::copy(png_data.data() + 12, png_data.data() + 16, chunk_type_bytes);
            
        //convertir a uint32_t
        uint32_t chunk_length = (chunk_length_bytes[0] << 24) | (chunk_length_bytes[1] << 16) | (chunk_length_bytes[2] << 8) | (chunk_length_bytes[3]);
            
        if (chunk_length == 0) {
            throw std::runtime_error("el tamano es 0 bld: " + filename);
            
        }

        if (std::equal(chunk_type_bytes, chunk_type_bytes + 4, "IHDR")) {
            //leer el chunk data
            uint8_t* chunk_data = png_data.data() + 16;
            result.width = (chunk_data[0] << 24) | (chunk_data[1] << 16) | (chunk_data[2] << 8) | (chunk_data[3]); // ancho 4 bytes
            result.height = (chunk_data[4] << 24) | (chunk_data[5] << 16) | (chunk_data[6] << 8) | (chunk_data[7]); // alto 4 bytes
            
        }
        //hasta ahora leimos 24 bytes
        //mover el puntero al siguiente chunk
        uint8_t bit_depth = png_data[24]; 
        uint8_t colour_type = png_data[25];
        uint8_t compression_method = png_data[26];
        uint8_t filter_method = png_data[27];
        uint8_t interlace_method = png_data[28];
        uint8_t color_channels = 0;
        if(colour_type == 0) color_channels = 1; //escala de grises
        if(colour_type == 2) color_channels = 3; //RGB
        if(colour_type == 3) color_channels = 1; //paleta de colores ya especificada
        if(colour_type == 4) color_channels = 2; //escala de grises + alpha
        if(colour_type == 6) color_channels = 4; //RGBA

        uint8_t bytes_per_pixel = (color_channels * bit_depth + 7) / 8; 
        //El +7 es para redondear hacia arriba, ya q al convertir a uint8_t se redondea para abajo

        if(interlace_method != 0){
            throw std::runtime_error("No soy tan mazoquista, off los q estan entrelazados: " + filename);
        }
        //ahora 28 bytes
        size_t stride = result.width * bytes_per_pixel;
        size_t raw_row_size = 1 + stride; 
        //dps de esto viene el CRC q no nos importa, entonces:
        size_t offset = 33; 
        
        //buffer para guardar todo lo q leemos
        std::vector<uint8_t> buffer;

        while(offset < png_data.size()){ //Mientras no terminemos de leer todo el archivo
            //Leer longitud de chunk 4 bytes
            uint32_t chunk_length = (png_data[offset] << 24) | (png_data[offset + 1] << 16) | (png_data[offset + 2] << 8) | png_data[offset + 3]; 
            
            offset +=4;

            //Leer tipo de chunk 4 bytes
            char chunk_type[5] = {
                (char)png_data[offset],     (char)png_data[offset + 1], 
                (char)png_data[offset + 2], (char)png_data[offset + 3], 0
            };
            offset += 4;

            // Procesar segun el tipo de chunk
            if (strcmp(chunk_type, "PLTE") == 0) {
                //cada paleta es solo RGB no ARGB
                //esto piko es un show mas referencia????, papaleta
                struct RGB { uint8_t r, g, b; };

                std::vector<RGB> palette;

                size_t num_colors = chunk_length / 3;
                
                palette.resize(num_colors);
    
                for (size_t i = 0; i < num_colors; ++i) {
                palette[i].r = png_data[offset + (i * 3)];
                palette[i].g = png_data[offset + (i * 3) + 1];
                palette[i].b = png_data[offset + (i * 3) + 2];
                }

                std::vector<uint8_t> final_rgb_pixels(result.width * result.height * 3);

                for (size_t i = 0; i < total_pixels; ++i) {
                    uint8_t palette_index = decompressed_and_defiltered_data[i];
    
                    // checkeo para saber si se corrompio el png
                    if (palette_index < palette.size()) {
                    final_rgb_pixels[i * 3]     = palette[palette_index].r;
                    final_rgb_pixels[i * 3 + 1] = palette[palette_index].g;
                    final_rgb_pixels[i * 3 + 2] = palette[palette_index].b;
                    }
                }
            }
            if (strcmp(chunk_type, "IDAT") == 0) {
                // Insertar los bytes de datos de este IDAT al final del buffer
                compressed_idat_buffer.insert(
                    compressed_idat_buffer.end(),
                    &png_data[offset],
                    &png_data[offset + chunk_length]
                );
            }
            else if (strcmp(chunk_type, "IEND") == 0) {
                // fin del archivo png
                break;
            }

            // saltar los datos del chunk (si no es IDAT) 
            offset += chunk_length + 4;
        }

        }   
        

        //ahora toca loopear de verdad



        
    }
    return result;
}

int main(){
    image_file img = read_image("test.png");
    std::cout << "Width: " << img.width << ", Height: " << img.height << std::endl;
    return 0;
}

