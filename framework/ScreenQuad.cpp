
#include "ScreenQuad.hpp"


ScreenQuad::ScreenQuad(){

    glGenVertexArrays(1,&quadVAO);
    glGenBuffers(1,&quadVBO);
    glGenBuffers(1,&quadTBO);
    glGenBuffers(1,&quadEBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


    // 4. then set the vertex attributes pointers
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  

    glBindBuffer(GL_ARRAY_BUFFER, quadTBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);  

    //  3. copy our index array in a element buffer for OpenGL to use
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

}



void ScreenQuad::render(){

    static ScreenQuad instance{};

    // quadShader.Use();
    glBindVertexArray(instance.quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER,instance.quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER,instance.quadTBO);
    glBindBuffer(GL_ARRAY_BUFFER,instance.quadEBO);
    glDisable(GL_DEPTH_TEST);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);

}
