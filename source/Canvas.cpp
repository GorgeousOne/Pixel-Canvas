#include "Canvas.h"
#include <sstream>    // std::stringstream
#include "utils.hpp"

const std::string canvasVertPath("../../../source/shader/image_display.vert");
const std::string canvasFragPath("../../../source/shader/image_display.frag");

const std::string wirenetVertPath("../../../source/shader/wirenet.vert");
const std::string wirenetFragPath("../../../source/shader/wirenet.frag");


//TODO move to utils
GLuint loadShaders(std::string const &vs, std::string const &fs) {
    std::string v = readFile(vs);
    std::string f = readFile(fs);
    return createProgram(v, f);
}

/**
 * Returns all file names with a certain extension in a directory
 */
std::vector<fs::path> getAllFiles(fs::path const &root, std::string const &ext) {
    std::vector<fs::path> paths;

    if (fs::exists(root) && fs::is_directory(root)) {
        for (auto const &entry: fs::recursive_directory_iterator(root)) {
            if (fs::is_regular_file(entry) && entry.path().extension() == ext) {
                paths.emplace_back(entry.path().filename());
            }
        }
    }
    return paths;
}

Canvas::Canvas() :
        visualizationFileNames(),
        visualizationDirs(),
        visualizationIndex(0),
        imgIndex(0),
        canvasShader(0),
        wirenetShader(0),
        screenScale(1, 1),
        zoom(0),
        pixelPos(0, 0) {

    canvas = createImgQuad();

    try {
        canvasShader = loadShaders(canvasVertPath, canvasFragPath);
        wirenetShader = loadShaders(wirenetVertPath, wirenetFragPath);
        std::cout << "loaded shaders " << canvasShader << " " << wirenetShader << std::endl;
    } catch (std::logic_error &e) {
        //std::cerr << e.what() << std::endl;
        std::stringstream ss;
        ss << e.what() << std::endl;
    }
}

void Canvas::display() {
    glm::mat4 viewMatrix = getViewMatrix();
    glm::mat4 canvasModelMatrix = getModelMatrix();
    //render canvas
    glUseProgram(canvasShader);
    GLuint modelLoc = glGetUniformLocation(canvasShader, "modelMatrix");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix * canvasModelMatrix));

    glBindVertexArray(canvas.vao);
    glBindTexture(GL_TEXTURE_2D, loadedImg.handle);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //draw little cursor box around focused pixel
    if (zoom > 2) {
        glUseProgram(wirenetShader);
        modelLoc = glGetUniformLocation(wirenetShader, "modelMatrix");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));

        glBindVertexArray(canvas.vao);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    }
}


/**
 * Creates matrix to scale screen to canvas pixel space
 */
glm::mat4 Canvas::getViewMatrix() const {
    return glm::scale(glm::mat4{}, glm::vec3(glm::vec2(glm::pow(2.0f, zoom)) / screenScale, 1.0f));
}

/**
 * Creates matrix that stretches 1x1 square to width x height pixels for image
 */
glm::mat4 Canvas::getModelMatrix() {
    //1 - (-1) = 2
    //idk scale camera to un-stretch window but also stretch canvas squad to image rect
    glm::vec2 canvasSize{loadedImg.width, loadedImg.height};

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(glm::mat4{}, glm::vec3(canvasSize, 1.0f)) * modelMatrix;
    glm::vec2 canvasPos = pixelPos;
    canvasPos.x *= -1;

    glm::vec2 screenPos = glm::round(canvasPos) + glm::vec2(-0.5, 0.5);

    modelMatrix = glm::translate(glm::mat4{}, glm::vec3(screenPos, 0.0f)) * modelMatrix; // Adjust xPosition and yPosition
//    modelMatrix = glm::scale(glm::mat4{}, glm::vec3(glm::vec2(glm::pow(2.0f, zoom)) / screenScale, 1.0f)) * modelMatrix;
    return modelMatrix;
}

/**
 * Creates a 1x1 unit square uploaded to opengl
 * @return
 */
Model Canvas::createImgQuad() {
    // Set up vertex data
    Model model;
    GLfloat vertices[] = {
            // Positions        // Texture Coords
            -0.5f,  0.5f,       0.0f, 0.0f,
            0.5f,  0.5f,       1.0f, 0.0f,
            0.5f, -0.5f,       1.0f, 1.0f,
            -0.5f, -0.5f,       0.0f, 1.0f
    };
    glGenVertexArrays(1, &model.vao);
    glGenBuffers(1, &model.vbo);

    glBindVertexArray(model.vao);
    glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return model;
}

void Canvas::setPixelPos(glm::vec2 newPos) {
    glm::vec2 border {loadedImg.width, loadedImg.height};
    pixelPos = glm::clamp(newPos, -0.5f * border, 0.5f * border - glm::vec2(1));
}

int Canvas::getPixelX() const {
    return (int) glm::round(pixelPos.x);
}

int Canvas::getPixelY() const {
    return (int) glm::round(pixelPos.x);
}

void Canvas::translate(glm::vec2 vec) {
    pixelPos += vec;
    glm::vec2 border {loadedImg.width, loadedImg.height};
    pixelPos = glm::clamp(pixelPos, -0.5f * border, 0.5f * border - glm::vec2(1));
}

glm::vec2 Canvas::getPixelPos(const glm::vec2& screenPos) const {
    // unstretch mouse pos and scale to canvas size
    glm::vec2 mousePos = screenPos * screenScale * 2.0f - screenScale;
    // apply zoom
    mousePos /= glm::pow(2.0f, zoom);
    // flip x idk
    mousePos.x *= -1;
    return mousePos;
}

void Canvas::loadImage(int i) {
    imgIndex = i;
    glDeleteTextures(1, &loadedImg.handle);
    loadedImg = texture_loader::uploadTexture(visualizationDirs[visualizationIndex] + visualizationFileNames[visualizationIndex][i].string());
}

void Canvas::reset() {
    zoom = 0;
    pixelPos = glm::vec2(0);
}

void Canvas::addZoom(float d) {
    zoom += d;
    zoom = glm::clamp(zoom, zoomMin, zoomMax);
}

int Canvas::getMaxImgIndex() {
    return visualizationFileNames[0].size() - 1;
}

void Canvas::updateScreenScale(glm::ivec2 windowSize) {
    float screenAspect = (float) windowSize.x / (float) windowSize.y;
    float canvasAspect = (float) loadedImg.width / (float) loadedImg.height;
    // adapt to either canvas width or height depending on whether window is wider or narrower than canvas
    if (screenAspect > canvasAspect) {
        screenScale = glm::vec2(0.5f * screenAspect * loadedImg.height, 0.5f * loadedImg.height);
    } else {
        screenScale = glm::vec2(0.5f * loadedImg.width, 0.5f * loadedImg.width / screenAspect);
    }
}

void Canvas::setImageDirs(std::vector<std::string> &dirList) {
    visualizationDirs = dirList;
    for (auto &dir : dirList) {
        visualizationFileNames.emplace_back(getAllFiles(dir, ".png"));
    }
    loadImage(imgIndex);
}

void Canvas::setVisualizationIndex(int i) {
    std::cout << "set visualization index " << i << std::endl;
    visualizationIndex = i;
    loadImage(imgIndex);
}
