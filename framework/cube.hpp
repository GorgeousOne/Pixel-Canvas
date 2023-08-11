#ifndef CUBE_HPP
#define CUBE_HPP

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

class Cube
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

  Cube();
  Cube(glm::vec3 min, glm::vec3 max);
  void draw() const;
  void freeVAO();

private:
  GLuint m_vao;
};

#endif // CUBE_HPP
