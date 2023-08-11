#ifndef SCREEN_QUAD_HPP
#define SCREEN_QUAD_HPP

#include <GL/glew.h>


class ScreenQuad
{
        // screen quad setup
    float vertices[8] = {
         1.0f,  1.0f,  // top right
         1.0f, -1.0f,  // bottom right
        -1.0f, -1.0f,  // bottom left
        -1.0f,  1.0f   // top left 
    };

    float texCoords[8] = {
         0.999999f,  0.9999f,  // top right
         0.9999f,  0.0f,  // bottom right
         0.0f,  0.0f,  // bottom left
         0.0f,  0.9999f   // top left 
    };
    unsigned int indices[6] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };  

    GLuint quadVAO = 0; 
    GLuint quadVBO = 0; 
    GLuint quadTBO = 0;
    GLuint quadEBO = 0;


    // singleton
    // prevent construction by user
    ScreenQuad();
    ScreenQuad(ScreenQuad const&) = delete;
    ScreenQuad& operator=(ScreenQuad const&) = delete;


public:
    static void render();
};



#endif