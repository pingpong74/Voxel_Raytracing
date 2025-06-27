#include <iostream>
#include <fstream>
#include <iomanip>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

union uint64_Mask {
    uint64_t mask;
    uint8_t bytes[8];
};

int main() {
    int width, height, channels;
    unsigned char* data = stbi_load("pal0.png", &width, &height, &channels, 3); // force RGB

    if (!data) {
        std::cerr << "Failed to load default.png\n";
        return 1;
    }

    if (width * height != 256) {
        std::cerr << "Expected 16x16 PNG (256 colors), got " << width << "x" << height << "\n";
        stbi_image_free(data);
        return 1;
    }

    std::cout << "const vec3 palette[256] = vec3[](\n";
    for (int i = 0; i < 256; ++i) {
        int base = i * 3;
        float r = data[base + 0] / 255.0f;
        float g = data[base + 1] / 255.0f;
        float b = data[base + 2] / 255.0f;
        std::cout << std::fixed << std::setprecision(4)
                  << "    vec3(" << r << ", " << g << ", " << b << ")";
        std::cout << (i < 255 ? "," : "") << "\n";
    }
    std::cout << ");\n";

    stbi_image_free(data);
    return 0;
}
