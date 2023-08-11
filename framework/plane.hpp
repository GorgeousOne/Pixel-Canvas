#ifndef PLANE_HPP
#define PLANE_HPP

// -----------------------------------------------------------------------------
// Copyright  : (C) 2014 Andreas-C. Bernstein
// License    : MIT (see the file LICENSE)
// Maintainer : Andreas-C. Bernstein <andreas.bernstein@uni-weimar.de>
// Stability  : experimental
//
// Cube
// -----------------------------------------------------------------------------

#include <GL/glew.h>

#ifdef __APPLE__
# define __gl3_h_
# define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#define GLM_FORCE_RADIANS
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class Plane
{
public:
  struct Vertex
  {
    Vertex()
      : position(0.0, 0.0, 0.0)
      , texCoord(0.0, 0.0)
    {}

    Vertex(glm::vec3 pos, glm::vec2 tc)
      : position(pos)
      , texCoord(tc)
    {}

    glm::vec3 position;
    glm::vec2 texCoord;
  };
    
  Plane(glm::vec2 min = glm::vec2(-0.0f), glm::vec2 max = glm::vec2(1.0f));
  void draw() const;

private:
    GLuint m_vao;
    GLuint m_ibo;
};

#endif // PLANE_HPP
