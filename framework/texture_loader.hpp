#ifndef TEXTURE_LOADER_HPP
#define TEXTURE_LOADER_HPP

#include <string>

struct Texture {
    GLuint handle;
    int width;
    int height;
    int channel_num;
};

namespace texture_loader {
    Texture uploadTexture(std::string const& file_name);
}

#endif