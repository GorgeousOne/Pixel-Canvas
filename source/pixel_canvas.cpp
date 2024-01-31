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
#include "utils.hpp"
#include "Canvas.h"

#include <string>
#include <iostream>
#include <filesystem>

///GLM INCLUDES
#define GLM_FORCE_RADIANS

#include <glm/gtx/transform.hpp>

///PROJECT INCLUDES
#include <imgui.h>

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

///IMGUI INCLUDES
#include <imgui_impl_glfw_gl3.h>
#include <vector>
#include <ctime>

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

//index of visualization image directory to currently render
int visualizationIndex = 0;

// index of currently displayed image
int currentImgIndex = 0;
// backup to determine if image index was changed
int lastImgIndex = -1;

// set backgorund color here
glm::vec3 g_background_color = glm::vec3(0.08f, 0.08f, 0.08f);   //grey
glm::ivec2 g_window_res = glm::ivec2(1920, 1080);
Window g_win(g_window_res);


// imgui variables
static bool mousePressed[2] = {false, false};

//imgui values
bool is_mouse_over_gui = false;
bool first_frame = true;
bool was_mouse_down = false;
bool is_canvas_pressed = false;

glm::vec2 lastMousePos{};

//determines how fine-grained zoom is when scrolling
float zoomStep = 0.2f;

// interval to number pictures with on slider
int imgMinuteInterval = 5;
// check if timelapse is running
bool isTimelapseRunning = false;
// images to advance per second during timelapse
float animationSpeed = 20;
// continuous image index for animation
float animationImgIndex = currentImgIndex;

//font settings
std::string fontPath = "../../../framework/extra_fonts/consola.ttf";
float fontSize = 24.0f;
float inputFieldWidth = 4 * fontSize;
ImVec2 spacing = ImVec2(fontSize, 0.5f * fontSize);

Canvas canvasObject{};

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
}

/**
 * Changes the focused pixel (which translates the canvas)
 */
void setPixelPos(glm::vec2 const& newPos) {
    canvasObject.setPixelPos(newPos);
}

/**
 * Prints error message if file does not exits
 */
void printPathExists(std::string path) {
    std::filesystem::path pathObj(path);
    if (!std::filesystem::exists(path)) {
        std::cerr << "Could not find path '" << path << "'\n";
    }
}

/**
 * Converts minutes to date format
 */
std::string convertToDate(int timeInMinutes) {
    std::time_t unixTimestamp = 1689858000 + timeInMinutes * 60; // Replace with your timestamp
    std::tm* timeInfo = localtime(&unixTimestamp);

    char formattedTime[80]; // A buffer to store the formatted date
    std::strftime(formattedTime, sizeof(formattedTime), "%b %d %H:%M", timeInfo);
    return formattedTime;
}

/**
 * Render all gui elements
 */
void showGUI(ImFont* font) {
    ImGui::PushFont(font);
    ImGui::Begin("Timeline:", nullptr);
    is_mouse_over_gui = ImGui::IsMouseHoveringAnyWindow();

    //timeline slider
    ImGui::Text("Timeline");
    bool didSliderMove = ImGui::SliderInt("##timeline-slider", &currentImgIndex, 0, canvasObject.getMaxImgIndex(), convertToDate(currentImgIndex * imgMinuteInterval).c_str());

    // stop animation when time slider clicked
    if (didSliderMove) {
        isTimelapseRunning = false;
    }
    // coordinate viewer / selector
    ImGui::Text("x:");
    ImGui::SameLine();

    ImGui::PushItemWidth(inputFieldWidth);

    //input fields for coordinates
    char xInputBuffer[32];

    int pixelX = canvasObject.getPixelX();
    int pixelY = canvasObject.getPixelY();

    snprintf(xInputBuffer, sizeof(xInputBuffer), "%d", pixelX);
    bool xChanged = ImGui::InputText("##ValueInput", xInputBuffer, sizeof(xInputBuffer), ImGuiInputTextFlags_CharsDecimal);

    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::Text("y:");
    ImGui::SameLine();
    ImGui::PushItemWidth(inputFieldWidth);

    char yInputBuffer[32]; // Buffer for y input text
    snprintf(yInputBuffer, sizeof(yInputBuffer), "%d", pixelY);
    bool yChanged = ImGui::InputText("##YInput", yInputBuffer, sizeof(yInputBuffer), ImGuiInputTextFlags_CharsDecimal);
    ImGui::PopItemWidth();

    // If user edited the input text, update the value
    if (xChanged) {
        setPixelPos(glm::vec2(atoi(xInputBuffer), pixelX));
    }
    if (yChanged) {
        setPixelPos(glm::vec2(pixelX, atoi(yInputBuffer)));
    }

    //animation speed
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << animationSpeed << "x";

    ImGui::Text("Animation speed:");
    ImGui::SliderFloat("##SpeedSlider", &animationSpeed, 4, 100, oss.str().c_str());

    //visualization radio button selection
    if (ImGui::RadioButton("Normal", &visualizationIndex, 0)) {
        canvasObject.setVisualizationIndex(0);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Brightness", &visualizationIndex, 1)) {
        canvasObject.setVisualizationIndex(1);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Thermal", &visualizationIndex, 2)) {
        canvasObject.setVisualizationIndex(2);
    }
    ImGui::End();
    ImGui::PopFont();
}

void handleUIInput() {
    if (!first_frame) {
        // exit window with escape
        if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) {
            g_win.stop();
        }
        //reset scaling and canvas movement
        if (ImGui::IsKeyPressed(GLFW_KEY_R)) {
            canvasObject.reset();
        }
        //move 1 image forward / backward
        if (ImGui::IsKeyPressed(GLFW_KEY_LEFT)) {
            isTimelapseRunning = false;
            currentImgIndex = glm::max(0, currentImgIndex - 1);
        }
        if (ImGui::IsKeyPressed(GLFW_KEY_RIGHT)) {
            isTimelapseRunning = false;
            currentImgIndex = glm::min(canvasObject.getMaxImgIndex(), currentImgIndex + 1);
        }

        //toggle timeline animation
        if (ImGui::IsKeyPressed(GLFW_KEY_SPACE, false)) {
            isTimelapseRunning = !isTimelapseRunning;
            animationImgIndex = currentImgIndex;

            //reset if at end of images
            if (isTimelapseRunning && currentImgIndex >= canvasObject.getMaxImgIndex()) {
                animationImgIndex = 0;
            }
        }
    }
}

void renderGUI(ImFont* font) {
    //IMGUI ROUTINE begin
    ImGuiIO &io = ImGui::GetIO();
    io.MouseWheel = 0;
    mousePressed[0] = mousePressed[1] = false;
    glfwPollEvents();
    updateImGui();

    // Start the frame
    //ImGui::NewFrame();
    ImGui_ImplGlfwGL3_NewFrame();
    showGUI(font);

    // Rendering
    //glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
    ImGui::Render();
    //IMGUI ROUTINE end
}

/**
 * Handles mouse user input
 * @param is_mouse_over_ui
 */
void handleMouse(bool is_mouse_over_ui) {
    // transform mouse position to canvas pixel space
    glm::vec2 mousePos = canvasObject.getPixelPos(g_win.mousePosition());
//    glm::vec2 mousePos = getMousePixelPos();

    // translate canvas on LMB drag
    if (ImGui::IsMouseDown(GLFW_MOUSE_BUTTON_LEFT)) {
        if (!was_mouse_down && !is_mouse_over_ui) {
            is_canvas_pressed = true;
        }
        if (is_canvas_pressed) {
            canvasObject.translate(mousePos - lastMousePos);
        }
        was_mouse_down = true;
    } else {
        was_mouse_down = false;
        is_canvas_pressed = false;
    }
    lastMousePos = mousePos;
}

/**
 * Handle mouse wheel input for zooming
 */
void handleMouseWheel(GLFWwindow* window, double xoffset, double yoffset) {
    canvasObject.addZoom(zoomStep * (float) yoffset);
}

std::vector<std::string> readImageDirectoryPaths() {
    if (!std::filesystem::exists("./resource_paths.txt")) {
        std::cerr << "Could not find ./resource_paths.txt\n";
    }
    std::vector<std::string> imageDirs;
    std::ifstream file("./resource_paths.txt");
    std::string imageParentDir;
    std::getline(file, imageParentDir);

    imageDirs.push_back(imageParentDir + "/default/");
    imageDirs.push_back(imageParentDir + "/heatmap/");
    imageDirs.push_back(imageParentDir + "/thermal_map/");
    return imageDirs;
}

int main(int argc, char *argv[]) {
    //InitImGui();
    ImGui_ImplGlfwGL3_Init(g_win.getGLFWwindow(), true);
    glfwSetScrollCallback(g_win.getGLFWwindow(), handleMouseWheel);

    //TODO check if window size is correct
    glm::ivec2 windowSize = g_win.windowSize();

    canvasObject.setImageDirs(readImageDirectoryPaths());
    canvasObject.updateScreenScale(windowSize);

    double previousTime = glfwGetTime();

    //customize font
    printPathExists(fontPath.c_str());
    ImGuiIO &io = ImGui::GetIO();
    ImFont* bigFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize);
    ImGui::GetStyle().ItemSpacing = spacing;

    while (true) {
        double currentTime = glfwGetTime();
        double elapsedTime = currentTime - previousTime;

        // advance image during animation
        if (isTimelapseRunning) {
            animationImgIndex += (float) elapsedTime * animationSpeed;
            currentImgIndex = glm::clamp((int) animationImgIndex, 0, canvasObject.getMaxImgIndex());

            if (currentImgIndex >= canvasObject.getMaxImgIndex()) {
                isTimelapseRunning = false;
            }
        }
        previousTime = currentTime;

        //load new image if image index changed
        if (currentImgIndex != lastImgIndex) {
            canvasObject.loadImage(currentImgIndex);
            lastImgIndex = currentImgIndex;
        }
        //TODO add back in if window needs to be resizable
//        updateScreenScale(size);

        handleUIInput();
        handleMouse(is_mouse_over_gui);

        //clear the screen
        glViewport(0, 0, windowSize.x, windowSize.y);
        glClearColor(g_background_color.x, g_background_color.y, g_background_color.z, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        canvasObject.display();
        renderGUI(bigFont);

        glBindTexture(GL_TEXTURE_2D, 0);
        g_win.update();
        first_frame = false;
    }

    //ImGui::Shutdown();
    ImGui_ImplGlfwGL3_Shutdown();
    return 0;
}
