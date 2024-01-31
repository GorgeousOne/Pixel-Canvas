#ifndef CANVAS_H
#define CANVAS_H

#define GLM_FORCE_RADIANS

// GLM includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>


#include <string>
#include <filesystem>
#include <vector>

#include "texture_loader.hpp"

namespace fs = std::filesystem;


//TODO rename to Mesh
struct Model {
    GLuint vao;
    GLuint vbo;
};

class Canvas {
public:
    Canvas();
    void display();
    void setPixelPos(glm::vec2 pos);
    void translate(glm::vec2 vec);

    glm::vec2 getPixelPos(const glm::vec2& screenPos) const;
    int getPixelX() const;
    int getPixelY() const;

    void loadImage(int i);

    void reset();

    void addZoom(float d);

    int getMaxImgIndex();

    void updateScreenScale(glm::ivec2 windowSize);

    void setImageDirs(std::vector<std::string> dirList);

    void setVisualizationIndex(int i);

private:
    std::vector<std::string> visualizationDirs;
    std::vector<std::vector<fs::path>> visualizationFileNames;
    int visualizationIndex = 0;
    int imgIndex = 0;

    glm::vec2 screenScale;
    float zoom;
    //TODO pass as parameter
    float zoomMax = 6;
    float zoomMin = -0.4;

    glm::vec2 pixelPos;

    GLuint canvasShader;
    GLuint wirenetShader;

    Texture loadedImg;
    //TODO rename to
    Model canvas;


    glm::mat4 getViewMatrix() const;
    glm::mat4 getModelMatrix();
    static Model createImgQuad();

};


#endif //CANVAS_H
