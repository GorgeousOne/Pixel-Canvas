#ifndef FENSTERCHEN_TURNTABLE_HPP
#define FENSTERCHEN_TURNTABLE_HPP

// -----------------------------------------------------------------------------
// Copyright  : (C) 2014 Andreas-C. Bernstein
// License    : MIT (see the file LICENSE)
// Maintainer : Andreas-C. Bernstein <andreas.bernstein@uni-weimar.de>
// Stability  : experimental
//
// Turntable controller
// -----------------------------------------------------------------------------

#define GLM_FORCE_RADIANS
#include <glm/vec2.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Turntable
{
public:
  Turntable() :
    zoomScale(1.0f),
    panningScale(1.0f),
    m_zoom(0),
    m_rotation(0,0),
    m_panning(0,0)
  {}

  inline void rotate(glm::vec2 const& o,glm::vec2 const& e) {
    m_rotation += (e - o);
  }
  inline void pan(glm::vec2 const& o,glm::vec2 const& e) {
    m_panning += panningScale * (e - o);
  }
  inline void zoom(glm::vec2 const& o,glm::vec2 const& e) {
    m_zoom += zoomScale * (e.y - o.y);
  }

  inline glm::mat4 matrix() const {
      glm::vec4 rotate_x = glm::rotate(m_rotation.x, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1.0, 0.0f, 0.0f, 1.0f);
      return glm::translate(glm::vec3(0.0f, 0.0f, -m_zoom))
          * glm::translate(glm::vec3(m_panning.x, m_panning.y, 0.0f))
          * glm::rotate(m_rotation.x, glm::vec3(0.0f, 1.0f, 0.0f))
          * glm::rotate(-m_rotation.y, glm::vec3(rotate_x.x, 0.0f, -rotate_x.z))
          ;
  }

  float       zoomScale;
  float       panningScale;
private:
  float       m_zoom;
  glm::vec2   m_rotation;
  glm::vec2   m_panning;
};

#endif // #ifndef FENSTERCHEN_TURNTABLE_HPP
