#include <iostream>
#include "texture_loader.hpp"
#include "GL/glew.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_LINEAR
#define STBI_FAILURE_USERMSG

#include "stb_image.h"

namespace texture_loader {
    Texture uploadTexture(std::string const &file_name) {
        Texture tex{};
        unsigned char *data = stbi_load(file_name.c_str(), &tex.width, &tex.height, &tex.channel_num, STBI_rgb);

        glGenTextures(1, &tex.handle);

        glBindTexture(GL_TEXTURE_2D, tex.handle);
        //2d texture, no mip map, internal format on GPU, w, h, border 0, format of original image, it's data type, pointer to texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.width, tex.height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        //keep clean pixels but smooth if zooming out
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        stbi_image_free(data);
        return tex;
    }
}