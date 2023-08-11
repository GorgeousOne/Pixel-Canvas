// -----------------------------------------------------------------------------
// Copyright  : (C) 2015 Sebastian Thiele
// License    : MIT (see the file LICENSE)
// Maintainer : Sebastian Thiele <sebastian.thiele@uni-weimar.de>
// Stability  : experimental
//
// Plane
// -----------------------------------------------------------------------------

#include "plane.hpp"
#include <array>

namespace {

std::array<Plane::Vertex, 4> get_cubeVertices(glm::vec2 min, glm::vec2 max) {

  std::array<Plane::Vertex, 4> vertex_array;

  // bottom
  vertex_array[0] = Plane::Vertex(glm::vec3(min.x, min.y, 0.0f), glm::vec2(0.0f, 0.0f));
  vertex_array[1] = Plane::Vertex(glm::vec3(max.x, min.y, 0.0f), glm::vec2(1.0f, 0.0f));
  vertex_array[2] = Plane::Vertex(glm::vec3(min.x, max.y, 0.0f), glm::vec2(0.0f, 1.0f));
  vertex_array[3] = Plane::Vertex(glm::vec3(max.x, max.y, 0.0f), glm::vec2(1.0f, 1.0f));

  return vertex_array;
  
    }
}

Plane::Plane(glm::vec2 min, glm::vec2 max)
  : m_vao(0),
  m_ibo(0)
{
  std::array<Plane::Vertex, 4> planeVertices = get_cubeVertices(min, max);

  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, sizeof(float)* 5 * planeVertices.size()
      , planeVertices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
  
  // generate and bind the index buffer object
  glGenBuffers(1, &m_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

  GLuint indexData[] = {
      0, 1, 2, // first triangle
      2, 1, 3, // second triangle
  };

  // fill with data
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* 2 * 3, indexData, GL_STATIC_DRAW);

  // "unbind" vao
  glBindVertexArray(0);
}

void Plane::draw() const
{
  glBindVertexArray(m_vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
  
  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
      //std::cerr << error << std::endl;
      return;
  }
}
