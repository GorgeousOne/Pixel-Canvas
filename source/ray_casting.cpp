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

#define SHOW_TRANSFER_FUNCTION_WINDOW 1

#define _USE_MATH_DEFINES

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

///PROJECT INCLUDES
#include <utils.hpp>
#include <imgui.h>

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

///IMGUI INCLUDES
#include <imgui_impl_glfw_gl3.h>
#include <vector>


//-----------------------------------------------------------------------------
// Helpers
//-----------------------------------------------------------------------------

#define IM_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR)/sizeof(*_ARR)))

#undef PI
const float PI = 3.14159265358979323846f;

#ifdef INT_MAX
#define IM_INT_MAX INT_MAX
#else
#define IM_INT_MAX 2147483647
#endif

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
int lastImgIndex = currentImgIndex;
std::string timelineImgDir = "C:/Users/Fred Feuerpferd/git-repos/Vis-Project/data/real_timeline/";
std::vector<fs::path> timelineFiles;
Texture loadedImg;

// set backgorund color here
glm::vec3 g_background_color = glm::vec3(0.08f, 0.08f, 0.08f);   //grey

glm::ivec2 g_window_res = glm::ivec2(1280, 720);
Window g_win(g_window_res);

// Volume Rendering GLSL Program
GLuint shaderProgram(0);
std::string g_error_message;
bool g_reload_shader_error = false;

// imgui variables
static bool g_show_gui = true;
static bool mousePressed[2] = {false, false};

//imgui values
bool g_over_gui = false;
bool g_reload_shader = false;
bool g_reload_shader_pressed = false;


bool g_pause = false;

bool first_frame = true;

void UpdateImGui() {
    ImGuiIO &io = ImGui::GetIO();

    // Setup resolution (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(g_win.getGLFWwindow(), &w, &h);
    glfwGetFramebufferSize(g_win.getGLFWwindow(), &display_w, &display_h);
    io.DisplaySize = ImVec2((float) display_w, (float) display_h);                                   // Display size, in pixels. For clamping windows positions.

    // Setup time step
    static double time = 0.0f;
    const double current_time = glfwGetTime();
    io.DeltaTime = (float) (current_time - time);
    time = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    double mouse_x, mouse_y;
    glfwGetCursorPos(g_win.getGLFWwindow(), &mouse_x, &mouse_y);
    mouse_x *= (float) display_w / w;                                                               // Convert mouse coordinates to pixels
    mouse_y *= (float) display_h / h;
    io.MousePos = ImVec2((float) mouse_x, (float) mouse_y);                                          // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
    io.MouseDown[0] = mousePressed[0] || glfwGetMouseButton(g_win.getGLFWwindow(), GLFW_MOUSE_BUTTON_LEFT) !=
                                         0;  // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
    io.MouseDown[1] = mousePressed[1] || glfwGetMouseButton(g_win.getGLFWwindow(), GLFW_MOUSE_BUTTON_RIGHT) != 0;

    // Start the frame
    //ImGui::NewFrame();
    ImGui_ImplGlfwGL3_NewFrame();
}


std::vector<fs::path> get_all(fs::path const &root, std::string const &ext) {
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
    ImVec2 whole_window_size = io.DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGui::SetNextWindowSize(ImVec2(400, whole_window_size.y));
    ImGui::Begin("Timeline", &g_show_gui);
    g_over_gui = ImGui::IsMouseHoveringAnyWindow();

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
    ImGui::SliderInt("Image Index", &currentImgIndex, 0, timelineFiles.size() - 1);

    if (g_task_chosen != g_task_chosen_old) {
        g_reload_shader = true;
        g_task_chosen_old = g_task_chosen;
    }

    if (ImGui::CollapsingHeader("Shader", 0, true, true)) {
        static ImVec4 text_color(1.0, 1.0, 1.0, 1.0);

        if (g_reload_shader_error) {
            text_color = ImVec4(1.0, 0.0, 0.0, 1.0);
            ImGui::TextColored(text_color, "Shader Error");
        } else {
            text_color = ImVec4(0.0, 1.0, 0.0, 1.0);
            ImGui::TextColored(text_color, "Shader Ok");
        }
        ImGui::TextWrapped(g_error_message.c_str());
        g_reload_shader ^= ImGui::Button("Reload Shader");
    }
    ImGui::End();
}

void handleUIInput() {
    if (!first_frame) {
        // exit window with escape
        if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) {
            g_win.stop();
        }

        if (ImGui::IsKeyPressed(GLFW_KEY_R)) {
            g_reload_shader = true;
        }

        if (ImGui::IsMouseDown(GLFW_MOUSE_BUTTON_LEFT) || ImGui::IsMouseDown(GLFW_MOUSE_BUTTON_MIDDLE) || ImGui::IsMouseDown(GLFW_MOUSE_BUTTON_RIGHT)) {
        }
    }
    if (ImGui::IsKeyPressed(GLFW_KEY_R)) {
        if (!g_reload_shader_pressed) {
            g_reload_shader = true;
            g_reload_shader_pressed = true;
        } else {
            g_reload_shader = false;
        }
    } else {
        g_reload_shader = false;
        g_reload_shader_pressed = false;
    }
}

void reloadShaders() {
    GLuint newProgram(0);
    try {
        //std::cout << "Reload shaders" << std::endl;
        newProgram = loadShaders(g_file_vertex_shader, g_file_fragment_shader);
        g_error_message = "";
    }
    catch (std::logic_error &e) {
        //std::cerr << e.what() << std::endl;
        std::stringstream ss;
        ss << e.what() << std::endl;
        g_error_message = ss.str();
        g_reload_shader_error = true;
        newProgram = 0;
    }
    if (0 != newProgram) {
        glDeleteProgram(shaderProgram);
        shaderProgram = newProgram;
        g_reload_shader_error = false;

    } else {
        g_reload_shader_error = true;
    }
}

void renderGUI() {
    //IMGUI ROUTINE begin
    ImGuiIO &io = ImGui::GetIO();
    io.MouseWheel = 0;
    mousePressed[0] = mousePressed[1] = false;
    glfwPollEvents();
    UpdateImGui();
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

Model createQuad() {
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

glm::vec2 canvasOffset{};
glm::vec2 lastMousePos{};

void handleMouse(float aspect) {
    glm::vec2 mousePos = g_win.mousePosition();

    if (ImGui::IsMouseDown(GLFW_MOUSE_BUTTON_LEFT)) {
        canvasOffset += (mousePos - lastMousePos) * glm::vec2(aspect, 1.0);
    }
    if (ImGui::IsMouseDown(GLFW_MOUSE_BUTTON_MIDDLE)) {
    }
    lastMousePos = mousePos;
}

glm::mat4 getModelMatrix() {
    return glm::mat4{};
}

void updateImage() {
    if (currentImgIndex != lastImgIndex) {
        glDeleteTextures(1, &loadedImg.handle);
        loadedImg = texture_loader::uploadTexture(timelineImgDir + timelineFiles[currentImgIndex].string());
        lastImgIndex = currentImgIndex;
    }
}

int main(int argc, char *argv[]) {
    //InitImGui();
    ImGui_ImplGlfwGL3_Init(g_win.getGLFWwindow(), true);

    // loading actual raytracing shader code (volume.vert, volume.frag)
    // edit volume.frag to define the result of our volume raycaster
    try {
        shaderProgram = loadShaders(g_file_vertex_shader, g_file_fragment_shader);
    }
    catch (std::logic_error &e) {
        //std::cerr << e.what() << std::endl;
        std::stringstream ss;
        ss << e.what() << std::endl;
        g_error_message = ss.str();
        g_reload_shader_error = true;
    }

    timelineFiles = get_all(timelineImgDir, ".png");
    loadedImg = texture_loader::uploadTexture(timelineImgDir + timelineFiles[currentImgIndex].string());
    Model canvas = createQuad();

    // manage keys here
    // add new input if neccessary (ie changing sampling distance, isovalues, ...)
    while (!g_win.shouldClose()) {

        /// reload shader if key R ist pressed
        if (g_reload_shader) {
            reloadShaders();
        }
        updateImage();

        glm::ivec2 size = g_win.windowSize();
        float imgAspect = (float) loadedImg.width / (float) loadedImg.height;
        float screenAspect = (float) size.x / (float) size.y;

        handleUIInput();
        if (!g_over_gui) {
            handleMouse(screenAspect);
        }

        glViewport(0, 0, size.x, size.y);
        glClearColor(g_background_color.x, g_background_color.y, g_background_color.z, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //1 - (-1) = 2
        float scaleX = 2.0f;
        float scaleY = 2.0f;

        //idk scale camera to un-stretch window but also stretch canvas squad to image rect
        if (imgAspect / screenAspect > 1.0f) {
            scaleY /= imgAspect / screenAspect;
        } else {
            scaleX *= imgAspect / screenAspect;
        }

        //render canvas
        glUseProgram(shaderProgram);
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleX, scaleY, 1.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(canvasOffset, 0.0f)); // Adjust xPosition and yPosition

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
