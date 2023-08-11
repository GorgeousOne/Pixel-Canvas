// -----------------------------------------------------------------------------
// Copyright  : (C) 2014 Andreas-C. Bernstein
// License    : MIT (see the file LICENSE)
// Maintainer : Andreas-C. Bernstein <andreas.bernstein@uni-weimar.de>
// Stability  : experimental
//
// Cube
// -----------------------------------------------------------------------------

#include "cube.hpp"
#include <array>

namespace {

std::array<Cube::Vertex, 36> get_cubeVertices(glm::vec3 min, glm::vec3 max) {

  std::array<Cube::Vertex, 36> vertex_array;

  // bottom
  vertex_array[0] = Cube::Vertex( glm::vec3( min.x, min.y, min.z), glm::vec2( 0.0f, 0.0f));
  vertex_array[1] = Cube::Vertex( glm::vec3( max.x, min.y, min.z), glm::vec2( 1.0f, 0.0f));
  vertex_array[2] = Cube::Vertex( glm::vec3( min.x, min.y, max.z), glm::vec2( 0.0f, 1.0f));
  vertex_array[3] = Cube::Vertex( glm::vec3( max.x, min.y, min.z), glm::vec2( 1.0f, 0.0f));
  vertex_array[4] = Cube::Vertex( glm::vec3( max.x, min.y, max.z), glm::vec2( 1.0f, 1.0f));
  vertex_array[5] = Cube::Vertex( glm::vec3( min.x, min.y, max.z), glm::vec2( 0.0f, 1.0f));

  //top
  vertex_array[6] = Cube::Vertex( glm::vec3( min.x, max.y, min.z ), glm::vec2(0.0f, 0.0f ));
  vertex_array[7] = Cube::Vertex( glm::vec3( min.x, max.y, max.z ), glm::vec2(0.0f, 1.0f ));
  vertex_array[8] = Cube::Vertex( glm::vec3( max.x, max.y, min.z ), glm::vec2(1.0f, 0.0f ));
  vertex_array[9] = Cube::Vertex( glm::vec3( max.x, max.y, min.z ), glm::vec2(1.0f, 0.0f ));
  vertex_array[10] = Cube::Vertex( glm::vec3(min.x, max.y, max.z ), glm::vec2(0.0f, 1.0f ));
  vertex_array[11] = Cube::Vertex( glm::vec3(max.x, max.y, max.z ), glm::vec2(1.0f, 1.0f ));

  //front
  vertex_array[12] = Cube::Vertex( glm::vec3(min.x, min.y, max.z ), glm::vec2(1.0f, 0.0f ));
  vertex_array[13] = Cube::Vertex( glm::vec3(max.x, min.y, max.z ), glm::vec2(0.0f, 0.0f ));
  vertex_array[14] = Cube::Vertex( glm::vec3(min.x, max.y, max.z ), glm::vec2(1.0f, 1.0f ));
  vertex_array[15] = Cube::Vertex( glm::vec3(max.x, min.y, max.z ), glm::vec2(0.0f, 0.0f ));
  vertex_array[16] = Cube::Vertex( glm::vec3(max.x, max.y, max.z ), glm::vec2(0.0f, 1.0f ));
  vertex_array[17] = Cube::Vertex( glm::vec3(min.x, max.y, max.z ), glm::vec2(1.0f, 1.0f ));

  //back
  vertex_array[18] = Cube::Vertex( glm::vec3(min.x, min.y, min.z ), glm::vec2(0.0f, 0.0f ));
  vertex_array[19] = Cube::Vertex( glm::vec3(min.x, max.y, min.z ), glm::vec2(0.0f, 1.0f ));
  vertex_array[20] = Cube::Vertex( glm::vec3(max.x, min.y, min.z ), glm::vec2(1.0f, 0.0f ));
  vertex_array[21] = Cube::Vertex( glm::vec3(max.x, min.y, min.z ), glm::vec2(1.0f, 0.0f ));
  vertex_array[22] = Cube::Vertex( glm::vec3(min.x, max.y, min.z ), glm::vec2(0.0f, 1.0f ));
  vertex_array[23] = Cube::Vertex( glm::vec3(max.x, max.y, min.z ), glm::vec2(1.0f, 1.0f ));

  //left                                        .
  vertex_array[24] = Cube::Vertex( glm::vec3(min.x, min.y, max.z ), glm::vec2(0.0f, 1.0f ));
  vertex_array[25] = Cube::Vertex( glm::vec3(min.x, max.y, min.z ), glm::vec2(1.0f, 0.0f ));
  vertex_array[26] = Cube::Vertex( glm::vec3(min.x, min.y, min.z ), glm::vec2(0.0f, 0.0f ));
  vertex_array[27] = Cube::Vertex( glm::vec3(min.x, min.y, max.z ), glm::vec2(0.0f, 1.0f ));
  vertex_array[28] = Cube::Vertex( glm::vec3(min.x, max.y, max.z ), glm::vec2(1.0f, 1.0f ));
  vertex_array[29] = Cube::Vertex( glm::vec3(min.x, max.y, min.z ), glm::vec2(1.0f, 0.0f ));

  //right
  vertex_array[30] = Cube::Vertex( glm::vec3(max.x, min.y, max.z ), glm::vec2(1.0f, 1.0f ));
  vertex_array[31] = Cube::Vertex( glm::vec3(max.x, min.y, min.z ), glm::vec2(1.0f, 0.0f ));
  vertex_array[32] = Cube::Vertex( glm::vec3(max.x, max.y, min.z ), glm::vec2(0.0f, 0.0f ));
  vertex_array[33] = Cube::Vertex( glm::vec3(max.x, min.y, max.z ), glm::vec2(1.0f, 1.0f ));
  vertex_array[34] = Cube::Vertex( glm::vec3(max.x, max.y, min.z ), glm::vec2(0.0f, 0.0f ));
  vertex_array[35] = Cube::Vertex( glm::vec3(max.x, max.y, max.z ), glm::vec2(0.0f, 1.0f ));


  return vertex_array;
        //    // bottom
        //        Cube::Vertex{ glm::vec3{ min.x, min.y, min.z }, glm::vec2{ 0.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, min.y, min.z }, glm::vec2{ 1.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ min.x, min.y, max.z }, glm::vec2{ 0.0f, 1.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, min.y, min.z }, glm::vec2{ 1.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, min.y, max.z }, glm::vec2{ 1.0f, 1.0f } },
        //        Cube::Vertex{ glm::vec3{ min.x, min.y, max.z }, glm::vec2{ 0.0f, 1.0f } },

        //        // top
        //        Cube::Vertex{ glm::vec3{ min.x, max.y, min.z }, glm::vec2{ 0.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ min.x, max.y, max.z }, glm::vec2{ 0.0f, 1.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, max.y, min.z }, glm::vec2{ 1.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, max.y, min.z }, glm::vec2{ 1.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ min.x, max.y, max.z }, glm::vec2{ 0.0f, 1.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, max.y, max.z }, glm::vec2{ 1.0f, 1.0f } },
        //
        //        // front
        //        Cube::Vertex{ glm::vec3{ min.x, min.y, max.z }, glm::vec2{ 1.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, min.y, max.z }, glm::vec2{ 0.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ min.x, max.y, max.z }, glm::vec2{ 1.0f, 1.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, min.y, max.z }, glm::vec2{ 0.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, max.y, max.z }, glm::vec2{ 0.0f, 1.0f } },
        //        Cube::Vertex{ glm::vec3{ min.x, max.y, max.z }, glm::vec2{ 1.0f, 1.0f } },
        //
        //        // back
        //        Cube::Vertex{ glm::vec3{ min.x, min.y, min.z }, glm::vec2{ 0.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ min.x, max.y, min.z }, glm::vec2{ 0.0f, 1.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, min.y, min.z }, glm::vec2{ 1.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, min.y, min.z }, glm::vec2{ 1.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ min.x, max.y, min.z }, glm::vec2{ 0.0f, 1.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, max.y, min.z }, glm::vec2{ 1.0f, 1.0f } },
        //
        //        // left                     .
        //        Cube::Vertex{ glm::vec3{ min.x, min.y, max.z }, glm::vec2{ 0.0f, 1.0f } },
        //        Cube::Vertex{ glm::vec3{ min.x, max.y, min.z }, glm::vec2{ 1.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ min.x, min.y, min.z }, glm::vec2{ 0.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ min.x, min.y, max.z }, glm::vec2{ 0.0f, 1.0f } },
        //        Cube::Vertex{ glm::vec3{ min.x, max.y, max.z }, glm::vec2{ 1.0f, 1.0f } },
        //        Cube::Vertex{ glm::vec3{ min.x, max.y, min.z }, glm::vec2{ 1.0f, 0.0f } },
        //
        //        // right
        //        Cube::Vertex{ glm::vec3{ max.x, min.y, max.z }, glm::vec2{ 1.0f, 1.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, min.y, min.z }, glm::vec2{ 1.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, max.y, min.z }, glm::vec2{ 0.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, min.y, max.z }, glm::vec2{ 1.0f, 1.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, max.y, min.z }, glm::vec2{ 0.0f, 0.0f } },
        //        Cube::Vertex{ glm::vec3{ max.x, max.y, max.z }, glm::vec2{ 0.0f, 1.0f } }
        //};
    }
}

Cube::Cube()
  : m_vao(0)
{

  std::array<Cube::Vertex, 36> cubeVertices =
    get_cubeVertices(glm::vec3(-1.0f), glm::vec3(1.0f));

  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 5 * cubeVertices.size()
              , cubeVertices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), nullptr);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 5*sizeof(GLfloat),
      (GLvoid*)(3*sizeof(GLfloat)));
  glBindVertexArray(0);
}

Cube::Cube(glm::vec3 min, glm::vec3 max)
  : m_vao(0)
{
  std::array<Cube::Vertex, 36> cubeVertices = get_cubeVertices(min, max);

  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, sizeof(float)* 5 * cubeVertices.size()
      , cubeVertices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(GLfloat),
      (GLvoid*)(3 * sizeof(GLfloat)));
  glBindVertexArray(0);
}

void Cube::draw() const
{
  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}


void Cube::freeVAO()
{
    glDeleteVertexArrays(1, &m_vao);
}

