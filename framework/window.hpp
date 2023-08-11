#ifndef FENSTERCHEN_WINDOW_HPP
#define FENSTERCHEN_WINDOW_HPP

// -----------------------------------------------------------------------------
// Copyright  : (C) 2014 Andreas-C. Bernstein
// License    : MIT (see the file LICENSE)
// Maintainer : Andreas-C. Bernstein <andreas.bernstein@uni-weimar.de>
// Stability  : experimental
//
// Window
// -----------------------------------------------------------------------------

#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#endif

#include "color.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/multiple.hpp>

#include <array>
#include <string>

class Window
{
public:
  Window(glm::ivec2 const& windowsize = glm::ivec2(640, 480));
  ~Window();

  enum MouseButton
  {
    MOUSE_BUTTON_NONE   = 0,
    MOUSE_BUTTON_LEFT   = (1 << 0),
    MOUSE_BUTTON_RIGHT  = (1 << 1),
    MOUSE_BUTTON_MIDDLE = (1 << 2)
  };

  enum KeyAction
  {
    KEY_PRESS   = GLFW_PRESS,
    KEY_RELEASE = GLFW_RELEASE,
    KEY_REPEAT  = GLFW_REPEAT
  };

  void drawLine(glm::vec2 const& start,
                glm::vec2 const& end,
                Color const& color
                ) const;

  void drawLine(float startX, float startY,
                float endX, float endY,
                float r, float g, float b
                ) const;



  void drawPoint(glm::vec2 const& p, Color const& col) const;

  void drawPoint(float x, float y, float r, float g, float b) const;

  glm::vec2 mousePosition() const;

  bool shouldClose() const;
  void stop();
  void update();
  inline bool isKeyPressed(int key) const { return m_keypressed[key]; }
  inline bool isButtonPressed(MouseButton b) const {
    return 0 != (m_mouseButtonFlags & b);
  }
  glm::ivec2 windowSize() const;
  float getTime() const;

  GLFWwindow* getGLFWwindow() { return m_window; }

  void resize(glm::ivec2 const& windowsize = glm::ivec2(640, 480));

private:
  GLFWwindow* m_window;
  glm::ivec2 m_size;
  std::string const m_title;
  glm::vec2 m_mousePosition;

  int m_mouseButtonFlags;
  std::array<bool, 512> m_keypressed;

  static void cursorPositionCallback(GLFWwindow* win, double x, double y);
  static void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods);
  static void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods);
};

#endif // define FENSTERCHEN_WINDOW_HPP
