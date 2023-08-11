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
#include <string>
#include <iostream>
#include <sstream>      // std::stringstream
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <cmath>

///GLM INCLUDES
#define GLM_FORCE_RADIANS

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>

#include <glm/gtx/string_cast.hpp>

///PROJECT INCLUDES
#include <volume_loader_raw.hpp>
#include <transfer_function.hpp>
#include <utils.hpp>
#include <turntable.hpp>
#include <imgui.h>

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

///IMGUI INCLUDES
#include <imgui_impl_glfw_gl3.h>


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


int g_task_chosen = 0;
int g_task_chosen_old = g_task_chosen;
bool g_lighting_toggle = false;
bool g_binary_search_toggle = false;
bool g_gradient_volume_toggle = true;


const std::string g_file_vertex_shader("../../../source/shader/ray_casting.vert");
const std::string g_file_fragment_shader("../../../source/shader/ray_casting.frag");

void set_shader_define_value(std::string &shader_str, std::string const define_str, std::string const value) {
    int index = (int) shader_str.find(define_str);
    shader_str.replace(index + define_str.length() + 1, 1, value);
}

GLuint loadShaders(std::string const &vs, std::string const &fs) {
    std::string v = readFile(vs);
    std::string f = readFile(fs);
    set_shader_define_value(f, "#define TASK", std::to_string(g_task_chosen));
    set_shader_define_value(f, "#define ENABLE_LIGHTING", std::to_string(g_lighting_toggle));
    set_shader_define_value(f, "#define ENABLE_BINARY_SEARCH", std::to_string(g_binary_search_toggle));
    set_shader_define_value(f, "#define USE_GRADIENT_VOLUME", std::to_string(g_gradient_volume_toggle));
    return createProgram(v, f);
}

Turntable g_turntable;

///SETUP VOLUME RAYCASTER HERE
// set the volume file
std::string g_file_string = "../../../data/head_w256_h256_d225_c1_b8.raw";

// set the sampling distance for the ray traversal
float g_sampling_distance = 0.001f;
float g_sampling_distance_fact = 1.5f;
float g_sampling_distance_fact_move = 2.0f;
// float       g_sampling_distance_fact_ref = 1.0f;

float g_iso_value = 0.2f;
// float       g_iso_value_2 = 0.6f;

// set the light position and color for shading
glm::vec3 g_light_pos = glm::vec3(10.0, 10.0, 10.0);
glm::vec3 g_ambient_light_color = glm::vec3(0.7f, 0.7f, 0.7f);
glm::vec3 g_diffuse_light_color = glm::vec3(0.9f, 0.9f, 0.9f);
glm::vec3 g_specula_light_color = glm::vec3(1.f, 1.f, 1.f);
float g_ref_coef = 12.0;

// set backgorund color here
//glm::vec3   g_background_color = glm::vec3(1.0f, 1.0f, 1.0f); //white
glm::vec3 g_background_color = glm::vec3(0.08f, 0.08f, 0.08f);   //grey

glm::ivec2 g_window_res = glm::ivec2(1280, 720);
Window g_win(g_window_res);

// Volume Rendering GLSL Program
GLuint g_volume_program(0);
std::string g_error_message;
bool g_reload_shader_error = false;

Transfer_function g_transfer_fun;
int g_current_tf_data_value = 0;
GLuint g_transfer_texture;
bool g_transfer_dirty = true;
bool g_redraw_tf = true;

// imgui variables
static bool g_show_gui = true;
static bool mousePressed[2] = {false, false};

bool g_show_transfer_function_in_window = false;
glm::vec2 g_transfer_function_pos = glm::vec2(0.0f);
glm::vec2 g_transfer_function_size = glm::vec2(0.0f);

//imgui values
bool g_over_gui = false;
bool g_reload_shader = false;
bool g_reload_shader_pressed = false;
bool g_show_transfer_function = false;


bool g_pause = false;

Volume_loader_raw g_volume_loader;
volume_data_type g_volume_data;
glm::ivec3 g_vol_dimensions;
glm::vec3 g_max_volume_bounds;
unsigned g_channel_size = 0;
unsigned g_channel_count = 0;
GLuint g_volume_texture = 0;
GLuint g_gradient_volume_texture = 0;
Cube g_cube;

int g_bilinear_interpolation = true;

bool first_frame = true;

struct Manipulator {
    Manipulator()
            : m_turntable(), m_mouse_button_pressed(0, 0, 0), m_mouse(0.0f, 0.0f), m_lastMouse(0.0f, 0.0f) {}

    glm::mat4 matrix() {
        m_turntable.rotate(m_slidelastMouse, m_slideMouse);
        return m_turntable.matrix();
    }

    glm::mat4 matrix(Window const &g_win) {
        m_mouse = g_win.mousePosition();

        if (ImGui::IsMouseDown(GLFW_MOUSE_BUTTON_LEFT)) {
            if (!m_mouse_button_pressed[0]) {
                m_mouse_button_pressed[0] = 1;
            }
            m_turntable.rotate(m_lastMouse, m_mouse);
            m_slideMouse = m_mouse;
            m_slidelastMouse = m_lastMouse;
        } else {
            m_mouse_button_pressed[0] = 0;
            m_turntable.rotate(m_slidelastMouse, m_slideMouse);
            //m_slideMouse *= 0.99f;
            //m_slidelastMouse *= 0.99f;
        }

        if (ImGui::IsMouseDown(GLFW_MOUSE_BUTTON_MIDDLE)) {
            if (!m_mouse_button_pressed[1]) {
                m_mouse_button_pressed[1] = 1;
            }
            m_turntable.pan(m_lastMouse, m_mouse);
        } else {
            m_mouse_button_pressed[1] = 0;
        }

        if (ImGui::IsMouseDown(GLFW_MOUSE_BUTTON_RIGHT)) {
            if (!m_mouse_button_pressed[2]) {
                m_mouse_button_pressed[2] = 1;
            }
            m_turntable.zoom(m_lastMouse, m_mouse);
        } else {
            m_mouse_button_pressed[2] = 0;
        }

        m_lastMouse = m_mouse;
        return m_turntable.matrix();
    }

private:
    Turntable m_turntable;
    glm::ivec3 m_mouse_button_pressed;
    glm::vec2 m_mouse;
    glm::vec2 m_lastMouse;
    glm::vec2 m_slideMouse;
    glm::vec2 m_slidelastMouse;
};

bool read_volume() {

    //init volume g_volume_loader
    //Volume_loader_raw g_volume_loader;
    //read volume dimensions
    g_vol_dimensions = g_volume_loader.get_dimensions(g_file_string);

    g_sampling_distance = 1.0f / glm::max(glm::max(g_vol_dimensions.x, g_vol_dimensions.y), g_vol_dimensions.z);

    unsigned max_dim = std::max(std::max(g_vol_dimensions.x,
                                         g_vol_dimensions.y),
                                g_vol_dimensions.z);

    // calculating max volume bounds of volume (0.0 .. 1.0)
    g_max_volume_bounds = glm::vec3(g_vol_dimensions) / glm::vec3((float) max_dim);

    // loading volume file data
    g_volume_data = g_volume_loader.load_volume(g_file_string);
    g_channel_size = g_volume_loader.get_bit_per_channel(g_file_string) / 8;
    g_channel_count = g_volume_loader.get_channel_count(g_file_string);

    // setting up proxy geometry
    g_cube.freeVAO();
    g_cube = Cube(glm::vec3(0.0, 0.0, 0.0), g_max_volume_bounds);

    glActiveTexture(GL_TEXTURE0);
    g_volume_texture = createTexture3D(g_vol_dimensions.x, g_vol_dimensions.y, g_vol_dimensions.z, GL_RED, GL_UNSIGNED_BYTE, GL_RED, (char *) &g_volume_data[0]);

    return 0 != g_volume_texture;

}


uint32_t get_1d_index(const glm::ivec3 &idx_3d) {
    return (idx_3d.x + g_vol_dimensions.x * (idx_3d.y + idx_3d.z * g_vol_dimensions.y));
}

void create_gradient_volume_texture(const std::vector<glm::vec3> &gradient_volume) {
    glActiveTexture(GL_TEXTURE2);
    g_gradient_volume_texture = createTexture3D(g_vol_dimensions.x, g_vol_dimensions.y, g_vol_dimensions.z, GL_RGB32F, GL_FLOAT, GL_RGB, (const char *) gradient_volume.data());
}

void load_gradient_volume() {
    std::vector<glm::vec3> gradient_volume(g_vol_dimensions.x * g_vol_dimensions.y * g_vol_dimensions.z);

    // load in from file
    std::ifstream grad_vol_infile(g_file_string + ".grad", std::ios::binary);
    grad_vol_infile.read(reinterpret_cast<char *>(gradient_volume.data()), gradient_volume.size() * sizeof(glm::vec3));
    create_gradient_volume_texture(gradient_volume);
}

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


void showGUI() {

    ImGuiIO &io = ImGui::GetIO();
    ImVec2 whole_window_size = io.DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGui::SetNextWindowSize(ImVec2(400, whole_window_size.y));
    ImGui::Begin("Volume Settings", &g_show_gui);
    static float f;
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
    const float ms_per_frame_avg = ms_per_frame_accum / 120;

    if (ImGui::CollapsingHeader("Ray-casting Technique", 0, true, true)) {
        ImGui::RadioButton("Example: Average Intensity Projection", &g_task_chosen, 0);
        ImGui::RadioButton("Maximum Intensity Projection", &g_task_chosen, 1);
        ImGui::RadioButton("First-hit Iso-Surface Ray_casting", &g_task_chosen, 2);
        ImGui::RadioButton("Front-to-Back Compositing", &g_task_chosen, 5);

        ImGui::SetNextTreeNodeOpened(true);

        if (ImGui::TreeNode("Settings")) {

            ImGui::SliderFloat("Iso Value", &g_iso_value, 0.0f, 1.0f, "%.8f", 1.0f);
            g_reload_shader ^= ImGui::Checkbox("Enable Lighting", &g_lighting_toggle);
            g_reload_shader ^= ImGui::Checkbox("Enable Binary Search", &g_binary_search_toggle);
            g_reload_shader ^= ImGui::Checkbox("Use pre-calculated gradients", &g_gradient_volume_toggle);


            ImGui::TreePop();
        }

        if (g_task_chosen != g_task_chosen_old) {
            g_reload_shader = true;
            g_task_chosen_old = g_task_chosen;
        }
    }

    if (ImGui::CollapsingHeader("Load Volumes", 0, true, false)) {
        bool load_volume_1 = false;
        bool load_volume_2 = false;
        bool load_volume_3 = false;

        ImGui::Text("Volumes");
        load_volume_1 ^= ImGui::Button("Load Volume Head");
        load_volume_2 ^= ImGui::Button("Load Volume Engine");
        load_volume_3 ^= ImGui::Button("Load Volume Bucky");


        if (load_volume_1) {
            g_file_string = "../../../data/head_w256_h256_d225_c1_b8.raw";
            read_volume();
            load_gradient_volume();
        }
        if (load_volume_2) {
            g_file_string = "../../../data/Engine_w256_h256_d256_c1_b8.raw";
            read_volume();
            load_gradient_volume();
        }

        if (load_volume_3) {
            g_file_string = "../../../data/Bucky_uncertainty_data_w32_h32_d32_c1_b8.raw";
            read_volume();
            load_gradient_volume();
        }
    }
    if (ImGui::CollapsingHeader("Lighting Settings")) {
        ImGui::SliderFloat3("Position Light", &g_light_pos[0], -10.0f, 10.0f);

        ImGui::ColorEdit3("Ambient Color", &g_ambient_light_color[0]);
        ImGui::ColorEdit3("Diffuse Color", &g_diffuse_light_color[0]);
        ImGui::ColorEdit3("Specular Color", &g_specula_light_color[0]);

        ImGui::SliderFloat("Shininess coefficient", &g_ref_coef, 0.0f, 20.0f, "%.5f", 1.0f);


    }
    if (ImGui::CollapsingHeader("Quality Settings")) {
        // ImGui::Text("Interpolation");
        // ImGui::RadioButton("Nearest Neighbour", &g_bilinear_interpolation, 0);
        // ImGui::RadioButton("Bilinear", &g_bilinear_interpolation, 1);

        //ImGui::Text("Sampling Size");
        ImGui::SliderFloat("sampling distance factor", &g_sampling_distance_fact, 0.1f, 10.0f, "%.5f", 4.0f);
        //ImGui::SliderFloat("sampling factor move", &g_sampling_distance_fact_move, 0.1f, 10.0f, "%.5f", 4.0f);
        // ImGui::SliderFloat("reference sampling factor", &g_sampling_distance_fact_ref, 0.1f, 10.0f, "%.5f", 4.0f);
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

    if (ImGui::CollapsingHeader("Timing")) {
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", ms_per_frame_avg, 1000.0f / ms_per_frame_avg);

        float min = *std::min_element(ms_per_frame.begin(), ms_per_frame.end());
        float max = *std::max_element(ms_per_frame.begin(), ms_per_frame.end());

        if (max - min < 10.0f) {
            float mid = (max + min) * 0.5f;
            min = mid - 5.0f;
            max = mid + 5.0f;
        }
        static size_t values_offset = 0;

        char buf[50];
        sprintf(buf, "avg %f", ms_per_frame_avg);
        ImGui::PlotLines("Frame Times", &ms_per_frame.front(), (int) ms_per_frame.size(), (int) values_offset, buf, min - max * 0.1f, max * 1.1f, ImVec2(0, 70));

        ImGui::SameLine();
        ImGui::Checkbox("pause", &g_pause);
    }

    if (ImGui::CollapsingHeader("Window options")) {
        if (ImGui::TreeNode("Window Size")) {
            const char *items[] = {"640x480", "720x576", "1280x720", "1920x1080", "1920x1200", "2048x1536"};
            static int item2 = -1;
            bool press = ImGui::Combo("Window Size", &item2, items, IM_ARRAYSIZE(items));

            if (press) {
                glm::ivec2 win_re_size = glm::ivec2(640, 480);

                switch (item2) {
                    case 0:
                        win_re_size = glm::ivec2(640, 480);
                        break;
                    case 1:
                        win_re_size = glm::ivec2(720, 576);
                        break;
                    case 2:
                        win_re_size = glm::ivec2(1280, 720);
                        break;
                    case 3:
                        win_re_size = glm::ivec2(1920, 1080);
                        break;
                    case 4:
                        win_re_size = glm::ivec2(1920, 1200);
                        break;
                    case 5:
                        win_re_size = glm::ivec2(1920, 1536);
                        break;
                    default:
                        break;
                }
                g_win.resize(win_re_size);
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Background Color")) {
            ImGui::ColorEdit3("BC", &g_background_color[0]);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Style Editor")) {
            ImGui::ShowStyleEditor();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Logging")) {
            ImGui::LogButtons();
            ImGui::TreePop();
        }
    }
    ImGui::End();


#if SHOW_TRANSFER_FUNCTION_WINDOW
    ImGui::SetNextWindowPos(ImVec2(450, 50), ImGuiSetCond_FirstUseEver);

    g_show_transfer_function = ImGui::Begin("Transfer Function Window", &g_show_transfer_function_in_window, ImVec2(300, 500));

    g_transfer_function_pos.x = ImGui::GetItemBoxMin().x;
    g_transfer_function_pos.y = ImGui::GetItemBoxMin().y;

    g_transfer_function_size.x = ImGui::GetItemBoxMax().x - ImGui::GetItemBoxMin().x;
    g_transfer_function_size.y = ImGui::GetItemBoxMax().y - ImGui::GetItemBoxMin().y;

    static unsigned byte_size = 255;

    static ImVector<float> A;
    if (A.empty()) { A.resize(byte_size); }

    if (g_redraw_tf) {
        g_redraw_tf = false;

        image_data_type color_con = g_transfer_fun.get_RGBA_transfer_function_buffer();

        for (unsigned i = 0; i != byte_size; ++i) {
            A[i] = color_con[i * 4 + 3];
        }
    }
    ImGui::PlotLines("", &A.front(), (int) A.size(), (int) 0, "", 0.0, 255.0, ImVec2(0, 70));

    g_transfer_function_pos.x = ImGui::GetItemBoxMin().x;
    g_transfer_function_pos.y = ImGui::GetIO().DisplaySize.y - ImGui::GetItemBoxMin().y - 70;

    g_transfer_function_size.x = ImGui::GetItemBoxMax().x - ImGui::GetItemBoxMin().x;
    g_transfer_function_size.y = ImGui::GetItemBoxMax().y - ImGui::GetItemBoxMin().y;

    ImGui::SameLine();
    ImGui::Text("Color:RGB Plot: Alpha");

    static int data_value = 0;
    ImGui::SliderInt("Data Value", &data_value, 0, 255);
    static float col[4] = {0.4f, 0.7f, 0.0f, 0.5f};
    ImGui::ColorEdit4("color", col);
    bool add_entry_to_tf = false;
    add_entry_to_tf ^= ImGui::Button("Add entry");
    ImGui::SameLine();

    bool reset_tf = false;
    reset_tf ^= ImGui::Button("Reset");

    if (reset_tf) {
        g_transfer_fun.reset();
        g_transfer_dirty = true;
        g_redraw_tf = true;
    }

    if (add_entry_to_tf) {
        g_current_tf_data_value = data_value;
        g_transfer_fun.add((unsigned) data_value, glm::vec4(col[0], col[1], col[2], col[3]));
        g_transfer_dirty = true;
        g_redraw_tf = true;
    }

    if (ImGui::CollapsingHeader("Manipulate Values")) {
        Transfer_function::container_type con = g_transfer_fun.get_piecewise_container();
        bool delete_entry_from_tf = false;
        static std::vector<int> g_c_data_value;

        if (g_c_data_value.size() != con.size()) {
            g_c_data_value.resize(con.size());
        }
        int i = 0;

        for (Transfer_function::container_type::iterator c = con.begin(); c != con.end(); ++c) {
            int c_data_value = c->first;
            glm::vec4 c_color_value = c->second;

            g_c_data_value[i] = c_data_value;

            std::stringstream ss;
            if (c->first < 10)
                ss << c->first << "  ";
            else if (c->first < 100)
                ss << c->first << " ";
            else
                ss << c->first;

            bool change_value = false;
            change_value ^= ImGui::SliderInt(std::to_string(i).c_str(), &g_c_data_value[i], 0, 255);
            ImGui::SameLine();

            if (change_value) {
                if (con.find(g_c_data_value[i]) == con.end()) {
                    g_transfer_fun.remove(c_data_value);
                    g_transfer_fun.add((unsigned) g_c_data_value[i], c_color_value);
                    g_current_tf_data_value = g_c_data_value[i];
                    g_transfer_dirty = true;
                    g_redraw_tf = true;
                }
            }

            //delete
            bool delete_entry_from_tf = false;
            delete_entry_from_tf ^= ImGui::Button(std::string("Delete: ").append(ss.str()).c_str());

            if (delete_entry_from_tf) {
                g_current_tf_data_value = c_data_value;
                g_transfer_fun.remove(g_current_tf_data_value);
                g_transfer_dirty = true;
                g_redraw_tf = true;
            }

            static float n_col[4] = {0.4f, 0.7f, 0.0f, 0.5f};
            memcpy(&n_col, &c_color_value, sizeof(float) * 4);

            bool change_color = false;
            change_color ^= ImGui::ColorEdit4(ss.str().c_str(), n_col);

            if (change_color) {
                g_transfer_fun.add((unsigned) g_c_data_value[i], glm::vec4(n_col[0], n_col[1], n_col[2], n_col[3]));
                g_current_tf_data_value = g_c_data_value[i];
                g_transfer_dirty = true;
                g_redraw_tf = true;
            }

            ImGui::Separator();

            ++i;
        }
    }
    if (ImGui::CollapsingHeader("Transfer Function - Save/Load", 0, true, false)) {
        ImGui::Text("Transferfunctions");
        bool load_tf_1 = false;
        bool load_tf_2 = false;
        bool load_tf_3 = false;
        bool load_tf_4 = false;
        bool load_tf_5 = false;
        bool load_tf_6 = false;
        bool save_tf_1 = false;
        bool save_tf_2 = false;
        bool save_tf_3 = false;
        bool save_tf_4 = false;
        bool save_tf_5 = false;
        bool save_tf_6 = false;

        save_tf_1 ^= ImGui::Button("Save TF1");
        ImGui::SameLine();
        load_tf_1 ^= ImGui::Button("Load TF1");
        save_tf_2 ^= ImGui::Button("Save TF2");
        ImGui::SameLine();
        load_tf_2 ^= ImGui::Button("Load TF2");
        save_tf_3 ^= ImGui::Button("Save TF3");
        ImGui::SameLine();
        load_tf_3 ^= ImGui::Button("Load TF3");
        save_tf_4 ^= ImGui::Button("Save TF4");
        ImGui::SameLine();
        load_tf_4 ^= ImGui::Button("Load TF4");
        save_tf_5 ^= ImGui::Button("Save TF5");
        ImGui::SameLine();
        load_tf_5 ^= ImGui::Button("Load TF5");
        save_tf_6 ^= ImGui::Button("Save TF6");
        ImGui::SameLine();
        load_tf_6 ^= ImGui::Button("Load TF6");

        if (save_tf_1 || save_tf_2 || save_tf_3 || save_tf_4 || save_tf_5 || save_tf_6) {
            Transfer_function::container_type con = g_transfer_fun.get_piecewise_container();
            std::vector<Transfer_function::element_type> save_vect;

            for (Transfer_function::container_type::iterator c = con.begin(); c != con.end(); ++c) {
                save_vect.push_back(*c);
            }
            std::ofstream tf_file;
            if (save_tf_1) { tf_file.open("TF1.tf", std::ios::out | std::ofstream::binary); }
            if (save_tf_2) { tf_file.open("TF2.tf", std::ios::out | std::ofstream::binary); }
            if (save_tf_3) { tf_file.open("TF3.tf", std::ios::out | std::ofstream::binary); }
            if (save_tf_4) { tf_file.open("TF4.tf", std::ios::out | std::ofstream::binary); }
            if (save_tf_5) { tf_file.open("TF5.tf", std::ios::out | std::ofstream::binary); }
            if (save_tf_6) { tf_file.open("TF6.tf", std::ios::out | std::ofstream::binary); }

            //std::copy(save_vect.begin(), save_vect.end(), std::ostreambuf_iterator<char>(tf_file));
            tf_file.write((char *) &save_vect[0], sizeof(Transfer_function::element_type) * save_vect.size());
            tf_file.close();
        }
        if (load_tf_1 || load_tf_2 || load_tf_3 || load_tf_4 || load_tf_5 || load_tf_6) {
            Transfer_function::container_type con = g_transfer_fun.get_piecewise_container();
            std::vector<Transfer_function::element_type> load_vect;

            std::ifstream tf_file;

            if (load_tf_1) { tf_file.open("TF1.tf", std::ios::in | std::ifstream::binary); }
            if (load_tf_2) { tf_file.open("TF2.tf", std::ios::in | std::ifstream::binary); }
            if (load_tf_3) { tf_file.open("TF3.tf", std::ios::in | std::ifstream::binary); }
            if (load_tf_4) { tf_file.open("TF4.tf", std::ios::in | std::ifstream::binary); }
            if (load_tf_5) { tf_file.open("TF5.tf", std::ios::in | std::ifstream::binary); }
            if (load_tf_6) { tf_file.open("TF6.tf", std::ios::in | std::ifstream::binary); }

            if (tf_file.good()) {
                tf_file.seekg(0, tf_file.end);

                size_t size = tf_file.tellg();
                unsigned elements = (int) size / (unsigned) sizeof(Transfer_function::element_type);
                tf_file.seekg(0);

                load_vect.resize(elements);
                tf_file.read((char *) &load_vect[0], size);

                g_transfer_fun.reset();
                g_transfer_dirty = true;
                for (std::vector<Transfer_function::element_type>::iterator c = load_vect.begin(); c != load_vect.end(); ++c) {
                    g_transfer_fun.add(c->first, c->second);
                }
            }
            tf_file.close();
        }
    }
    ImGui::End();
#endif
}

void handleUIInput() {
    if (!first_frame > 0.0) {

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
        if (g_reload_shader_pressed != true) {
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
        glDeleteProgram(g_volume_program);
        g_volume_program = newProgram;
        g_reload_shader_error = false;

    } else {
        g_reload_shader_error = true;
    }
}

void uploadUniforms(glm::vec3 const & camera_location, float sampling_fact, glm::mat4 projection, glm::mat4 model_view) {
    glUniform1i(glGetUniformLocation(g_volume_program, "volume_texture"), 0);
    glUniform1i(glGetUniformLocation(g_volume_program, "transfer_func_texture"), 1);
    glUniform1i(glGetUniformLocation(g_volume_program, "gradient_volume_texture"), 2);

    glUniform3fv(glGetUniformLocation(g_volume_program, "camera_location"), 1, glm::value_ptr(camera_location));
    glUniform1f(glGetUniformLocation(g_volume_program, "sampling_distance"), g_sampling_distance * sampling_fact);
    // glUniform1f(glGetUniformLocation(g_volume_program, "sampling_distance_ref"), g_sampling_distance_fact_ref);
    glUniform1f(glGetUniformLocation(g_volume_program, "iso_value"), g_iso_value);
    glUniform3fv(glGetUniformLocation(g_volume_program, "max_bounds"), 1, glm::value_ptr(g_max_volume_bounds));
    glUniform3iv(glGetUniformLocation(g_volume_program, "volume_dimensions"), 1, glm::value_ptr(g_vol_dimensions));
    glUniform3fv(glGetUniformLocation(g_volume_program, "light_position"), 1, glm::value_ptr(g_light_pos));
    glUniform3fv(glGetUniformLocation(g_volume_program, "light_ambient_color"), 1, glm::value_ptr(g_ambient_light_color));
    glUniform3fv(glGetUniformLocation(g_volume_program, "light_diffuse_color"), 1, glm::value_ptr(g_diffuse_light_color));
    glUniform3fv(glGetUniformLocation(g_volume_program, "light_specular_color"), 1, glm::value_ptr(g_specula_light_color));
    glUniform1f(glGetUniformLocation(g_volume_program, "light_shininess"), g_ref_coef);

    glUniformMatrix4fv(glGetUniformLocation(g_volume_program, "Projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(g_volume_program, "Modelview"), 1, GL_FALSE, glm::value_ptr(model_view));
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

int main(int argc, char *argv[]) {
    //g_win = Window(g_window_res);
    //InitImGui();
    ImGui_ImplGlfwGL3_Init(g_win.getGLFWwindow(), true);

    // initialize the transfer function

    // first clear possible old values
    g_transfer_fun.reset();

    // the add_stop method takes:
    //  - unsigned char or float - data value     (0.0 .. 1.0) or (0..255)
    //  - vec4f         - color and alpha value   (0.0 .. 1.0) per channel
    g_transfer_fun.add(0.0f, glm::vec4(0.0, 0.0, 0.0, 0.0));
    g_transfer_fun.add(1.0f, glm::vec4(1.0, 1.0, 1.0, 1.0));
    g_transfer_dirty = true;


    // init and upload volume texture
    bool check = read_volume();

    load_gradient_volume();

    // init and upload transfer function texture
    glActiveTexture(GL_TEXTURE1);
    g_transfer_texture = createTexture2D(255u, 1u, (char *) &g_transfer_fun.get_RGBA_transfer_function_buffer()[0]);

    // loading actual raytracing shader code (volume.vert, volume.frag)
    // edit volume.frag to define the result of our volume raycaster
    try {
        g_volume_program = loadShaders(g_file_vertex_shader, g_file_fragment_shader);
    }
    catch (std::logic_error &e) {
        //std::cerr << e.what() << std::endl;
        std::stringstream ss;
        ss << e.what() << std::endl;
        g_error_message = ss.str();
        g_reload_shader_error = true;
    }

    // init object manipulator (turntable)
    Manipulator manipulator;

    // manage keys here
    // add new input if neccessary (ie changing sampling distance, isovalues, ...)
    while (!g_win.shouldClose()) {
        float sampling_fact = g_sampling_distance_fact;

        /// reload shader if key R ist pressed
        if (g_reload_shader) {
            reloadShaders();
        }
        handleUIInput();


        if (g_transfer_dirty && !first_frame) {
            g_transfer_dirty = false;
            static unsigned byte_size = 255;
            image_data_type color_con = g_transfer_fun.get_RGBA_transfer_function_buffer();
            glActiveTexture(GL_TEXTURE1);
            updateTexture2D(g_transfer_texture, 255u, 1u, (char *) &g_transfer_fun.get_RGBA_transfer_function_buffer()[0]);
        }

        if (g_bilinear_interpolation) {
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        glm::ivec2 size = g_win.windowSize();
        glViewport(0, 0, size.x, size.y);
        glClearColor(g_background_color.x, g_background_color.y, g_background_color.z, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float fovy = 45.0f;
        float aspect = (float) size.x / (float) size.y;
        float zNear = 0.025f, zFar = 10.0f;
        glm::mat4 projection = glm::perspective(fovy, aspect, zNear, zFar);

        glm::vec3 translate_rot = g_max_volume_bounds * glm::vec3(-0.5f, -0.5f, -0.5f);
        glm::vec3 translate_pos = g_max_volume_bounds * glm::vec3(+0.5f, -0.0f, -0.0f);

        glm::vec3 eye = glm::vec3(0.0f, 0.0f, 1.5f);
        glm::vec3 target = glm::vec3(0.0f);
        glm::vec3 up(0.0f, 1.0f, 0.0f);

        glm::mat4 view = glm::lookAt(eye, target, up);

        glm::mat4 turntable_matrix = manipulator.matrix();

        //rotate cube if not ui clicked
        if (!g_over_gui) {
            turntable_matrix = manipulator.matrix(g_win);
        }

        glm::mat4 model_view =
                view
                * glm::translate(translate_pos)
                * turntable_matrix
                // rotate head upright
                * glm::rotate(0.5f * float(M_PI), glm::vec3(0.0f, 1.0f, 0.0f))
                * glm::rotate(0.5f * float(M_PI), glm::vec3(1.0f, 0.0f, 0.0f))
                * glm::translate(translate_rot);


        glm::vec4 camera_translate = glm::column(glm::inverse(model_view), 3);
        glm::vec3 camera_location = glm::vec3(camera_translate.x, camera_translate.y, camera_translate.z);
        camera_location /= glm::vec3(camera_translate.w);

        glBindTexture(GL_TEXTURE_2D, g_transfer_texture);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glUseProgram(g_volume_program);
        uploadUniforms(camera_location, sampling_fact, projection, model_view);

        if (!g_pause) {
            g_cube.draw();
        }
        glUseProgram(0);
        renderGUI();

        if (g_show_transfer_function)
            g_transfer_fun.draw_texture(g_transfer_function_pos, g_transfer_function_size, g_transfer_texture);
        glBindTexture(GL_TEXTURE_2D, 0);

        g_win.update();
        first_frame = false;
    }

    //ImGui::Shutdown();
    ImGui_ImplGlfwGL3_Shutdown();
    return 0;
}
