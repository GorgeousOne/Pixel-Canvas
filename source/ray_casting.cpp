// -----------------------------------------------------------------------------
// Copyright  : (C) 2014 Andreas-C. Bernstein
//                  2015 Sebastian Thiele
// License    : MIT (see the file LICENSE)
// Maintainer : Sebastian Thiele <sebastian.thiele@uni-weimar.de>
// Stability  : experimantal exercise
//
// scivis exercise Example
// -----------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning (disable: 4996)         // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

#include "fensterchen.hpp"
#include "texture_loader.hpp"
#include "utils.hpp"

#include <string>
#include <iostream>
#include <sstream>      // std::stringstream
#include <stdexcept>
#include <filesystem>

///GLM INCLUDES
#define GLM_FORCE_RADIANS

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

///PROJECT INCLUDES
#include <imgui.h>

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

///IMGUI INCLUDES
#include <imgui_impl_glfw_gl3.h>
#include <vector>


//-----------------------------------------------------------------------------
// Helpers
//-----------------------------------------------------------------------------

#define IM_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR)/sizeof(*_ARR)))

// Play it nice with Windows users. Notepad in 2014 still doesn't display text data with Unix-style \n.
#ifdef _MSC_VER
#define STR_NEWLINE "\r\n"
#else
#define STR_NEWLINE "\n"
#endif

namespace fs = std::filesystem;

int g_task_chosen = 0;
int g_task_chosen_old = g_task_chosen;

const std::string g_file_vertex_shader("../../../source/shader/image_display.vert");
const std::string g_file_fragment_shader("../../../source/shader/image_display.frag");

GLuint loadShaders(std::string const &vs, std::string const &fs) {
    std::string v = readFile(vs);
    std::string f = readFile(fs);
    return createProgram(v, f);
}

int currentImgIndex = 0;
int lastImgIndex = -1;
std::string timelineImgDir = "C:/Users/Fred Feuerpferd/git-repos/Vis-Project/data/real_timeline/";
std::vector<fs::path> timelineFiles;
Texture loadedImg;

// set backgorund color here
glm::vec3 g_background_color = glm::vec3(0.08f, 0.08f, 0.08f);   //grey

glm::ivec2 g_window_res = glm::ivec2(1280, 720);
Window g_win(g_window_res);

GLuint shaderProgram(0);

// imgui variables
static bool g_show_gui = true;
static bool mousePressed[2] = {false, false};

//imgui values
bool is_mouse_over_gui = false;
bool g_reload_shader_pressed = false;


bool g_pause = false;
bool first_frame = true;

bool was_mouse_down = false;
bool is_canvas_pressed = false;

glm::vec2 screenSize{};
glm::vec2 pixelPos{};
glm::vec2 lastMousePos{};

float zoomMax = 5;
float zoomMin = 0;
float zoom = zoomMin;
float zoomStep = 1;

void updateImGui() {
    ImGuiIO &io = ImGui::GetIO();

    // Setup resolution (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(g_win.getGLFWwindow(), &w, &h);
    glfwGetFramebufferSize(g_win.getGLFWwindow(), &display_w, &display_h);
    // Display size, in pixels. For clamping windows positions.
    io.DisplaySize = ImVec2((float) display_w, (float) display_h);

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    double mouse_x, mouse_y;
    glfwGetCursorPos(g_win.getGLFWwindow(), &mouse_x, &mouse_y);
    // Convert mouse coordinates to pixels
    mouse_x *= (float) display_w / w;
    mouse_y *= (float) display_h / h;
    // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
    io.MousePos = ImVec2((float) mouse_x, (float) mouse_y);
    // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
    io.MouseDown[0] = mousePressed[0] || glfwGetMouseButton(g_win.getGLFWwindow(), GLFW_MOUSE_BUTTON_LEFT) != 0;
    io.MouseDown[1] = mousePressed[1] || glfwGetMouseButton(g_win.getGLFWwindow(), GLFW_MOUSE_BUTTON_RIGHT) != 0;

    // Start the frame
    //ImGui::NewFrame();
    ImGui_ImplGlfwGL3_NewFrame();
}

void setPixelPos(glm::vec2 const& newPos) {
    glm::vec2 border {loadedImg.width, loadedImg.height};
    pixelPos = glm::clamp(newPos, -0.5f * border, 0.5f * border - glm::vec2(1));
}

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

void showGUI() {
    ImGuiIO &io = ImGui::GetIO();

    int numTextLines = 10;
    float height = ImGui::GetTextLineHeightWithSpacing() * numTextLines;

    ImGui::SetNextWindowSize(ImVec2(400, height));
    ImGui::Begin("Timeline", nullptr);
    is_mouse_over_gui = ImGui::IsMouseHoveringAnyWindow();

    // Calculate and show frame rate
    static ImVector<float> ms_per_frame;

    if (ms_per_frame.empty()) {
        ms_per_frame.resize(400);
        memset(&ms_per_frame.front(), 0, ms_per_frame.size() * sizeof(float));
    }
    static int ms_per_frame_idx = 0;
    static float ms_per_frame_accum = 0.0f;
    if (!g_pause) {
        ms_per_frame_accum -= ms_per_frame[ms_per_frame_idx];
        ms_per_frame[ms_per_frame_idx] = ImGui::GetIO().DeltaTime * 1000.0f;
        ms_per_frame_accum += ms_per_frame[ms_per_frame_idx];

        ms_per_frame_idx = (ms_per_frame_idx + 1) % ms_per_frame.size();
    }
    //timeline slider
    ImGui::SliderInt("Image Index", &currentImgIndex, 0, timelineFiles.size() - 1);

    ImGui::Text("x");
    ImGui::SameLine();

    float inputFieldWidth = 50.0f;
    ImGui::PushItemWidth(inputFieldWidth);

    //input fields for coordinates
    char xInputBuffer[32];
    snprintf(xInputBuffer, sizeof(xInputBuffer), "%d", (int) pixelPos.x); // Convert int to string
    bool xChanged = ImGui::InputText("##ValueInput", xInputBuffer, sizeof(xInputBuffer), ImGuiInputTextFlags_CharsDecimal);

    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::Text("y");
    ImGui::SameLine();
    ImGui::PushItemWidth(inputFieldWidth);

    char yInputBuffer[32]; // Buffer for y input text
    snprintf(yInputBuffer, sizeof(yInputBuffer), "%d", (int) pixelPos.y);
    bool yChanged = ImGui::InputText("##YInput", yInputBuffer, sizeof(yInputBuffer), ImGuiInputTextFlags_CharsDecimal);
    ImGui::PopItemWidth();

    // If user edited the input text, update the value
    if (xChanged) {
        setPixelPos(glm::vec2(atoi(xInputBuffer), pixelPos.y));
    }
    if (yChanged) {
        setPixelPos(glm::vec2(pixelPos.x, atoi(yInputBuffer)));
    }
    ImGui::End();
}

void handleUIInput() {
    if (!first_frame) {
        // exit window with escape
        if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) {
            g_win.stop();
        }

        if (ImGui::IsMouseDown(GLFW_MOUSE_BUTTON_LEFT) || ImGui::IsMouseDown(GLFW_MOUSE_BUTTON_MIDDLE) || ImGui::IsMouseDown(GLFW_MOUSE_BUTTON_RIGHT)) {
        }
    }
}

void renderGUI() {
    //IMGUI ROUTINE begin
    ImGuiIO &io = ImGui::GetIO();
    io.MouseWheel = 0;
    mousePressed[0] = mousePressed[1] = false;
    glfwPollEvents();
    updateImGui();
    showGUI();

    // Rendering
    glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
    ImGui::Render();
    //IMGUI ROUTINE end
}

struct Model {
    GLuint vao;
    GLuint vbo;
};

Model createImgQuad() {
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

glm::vec2 getMousePixelPos() {
    glm::vec2 mousePos = g_win.mousePosition() * screenSize * 2.0f - screenSize;
    mousePos /= glm::pow(2.0f, zoom);
    mousePos.x *= -1;
    return mousePos;
}

void handleMouse(bool is_mouse_over_ui) {
    glm::vec2 mousePos = g_win.mousePosition() * screenSize * 2.0f / glm::pow(2.0f, zoom);
    mousePos.x *= -1;

    if (ImGui::IsMouseDown(GLFW_MOUSE_BUTTON_LEFT)) {
        if (!was_mouse_down && !is_mouse_over_ui) {
            is_canvas_pressed = true;
        }
        if (is_canvas_pressed) {
            setPixelPos(pixelPos + (mousePos - lastMousePos));
        }
        was_mouse_down = true;
    } else {
        was_mouse_down = false;
        is_canvas_pressed = false;
    }
    lastMousePos = mousePos;
}

void handleMouseWheel(GLFWwindow* window, double xoffset, double yoffset) {
    glm::vec2 delta = getMousePixelPos() - pixelPos;
    zoom = glm::clamp(zoom + zoomStep * (float) yoffset, zoomMin, zoomMax);
    glm::vec2 delta2 = getMousePixelPos() - pixelPos;

    //make zoom go to mouse pointer
    setPixelPos(pixelPos + delta2 - delta);
}

glm::mat4 getModelMatrix(float screenAspect) {
    //1 - (-1) = 2
    //idk scale camera to un-stretch window but also stretch canvas squad to image rect
    glm::vec2 canvasSize{loadedImg.width, loadedImg.height};

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(glm::mat4{}, glm::vec3(canvasSize, 1.0f)) * modelMatrix;
    glm::vec2 canvasPos = pixelPos;
    canvasPos.x *= -1;

    modelMatrix = glm::translate(glm::mat4{}, glm::vec3(glm::round(canvasPos), 0.0f)) * modelMatrix; // Adjust xPosition and yPosition
    modelMatrix = glm::scale(glm::mat4{}, glm::vec3(glm::vec2(glm::pow(2, zoom)) / screenSize, 1.0f)) * modelMatrix;

    return modelMatrix;
}

void updateImage() {
    if (currentImgIndex != lastImgIndex) {
        glDeleteTextures(1, &loadedImg.handle);
        loadedImg = texture_loader::uploadTexture(timelineImgDir + timelineFiles[currentImgIndex].string());

        float imgAspect = (float) loadedImg.width / (float) loadedImg.height;
        lastImgIndex = currentImgIndex;
    }
}

int main(int argc, char *argv[]) {
    //InitImGui();
    ImGui_ImplGlfwGL3_Init(g_win.getGLFWwindow(), true);
    glfwSetScrollCallback(g_win.getGLFWwindow(), handleMouseWheel);

    // loading actual raytracing shader code (volume.vert, volume.frag)
    // edit volume.frag to define the result of our volume raycaster
    try {
        shaderProgram = loadShaders(g_file_vertex_shader, g_file_fragment_shader);
    }
    catch (std::logic_error &e) {
        //std::cerr << e.what() << std::endl;
        std::stringstream ss;
        ss << e.what() << std::endl;
    }

    timelineFiles = getAllFiles(timelineImgDir, ".png");
    loadedImg = texture_loader::uploadTexture(timelineImgDir + timelineFiles[currentImgIndex].string());
    Model canvas = createImgQuad();

    // manage keys here
    // add new input if neccessary (ie changing sampling distance, isovalues, ...)
    while (!g_win.shouldClose()) {

        /// reload shader if key R ist pressed
        updateImage();

        glm::ivec2 size = g_win.windowSize();
        float screenAspect = (float) size.x / (float) size.y;
        float canvasAspect = (float) loadedImg.width / (float) loadedImg.height;
        // make screen scaling fit canvas
        if (screenAspect > canvasAspect) {
            screenSize = glm::vec2(0.5f * screenAspect * loadedImg.height, 0.5f * loadedImg.height);
        } else {
            screenSize = glm::vec2(0.5f * loadedImg.width, 0.5f * loadedImg.width / screenAspect);
        }

        handleUIInput();
        handleMouse(is_mouse_over_gui);
        glm::mat4 modelMatrix = getModelMatrix(screenAspect);

        glViewport(0, 0, size.x, size.y);
        glClearColor(g_background_color.x, g_background_color.y, g_background_color.z, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //render canvas
        glUseProgram(shaderProgram);
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "modelMatrix");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        glBindVertexArray(canvas.vao);
        glBindTexture(GL_TEXTURE_2D, loadedImg.handle);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glBindVertexArray(0);
        glUseProgram(0);
        renderGUI();

        glBindTexture(GL_TEXTURE_2D, 0);
        g_win.update();
        first_frame = false;
    }

    //ImGui::Shutdown();
    ImGui_ImplGlfwGL3_Shutdown();
    return 0;
}
