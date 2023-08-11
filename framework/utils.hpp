#ifndef UTILS_HPP
#define UTILS_HPP

// -----------------------------------------------------------------------------
// Copyright  : (C) 2014 Andreas-C. Bernstein
// License    : MIT (see the file LICENSE)
// Maintainer : Andreas-C. Bernstein <andreas.bernstein@uni-weimar.de>
// Stability  : experimental
//
// utils
// -----------------------------------------------------------------------------

#include <GL/glew.h>
#ifdef __APPLE__
# define __gl3_h_
# define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include <string>
#include <fstream>
#include <streambuf>
#include <cerrno>
#include <iostream>

// Read a small text file.
inline std::string readFile(std::string const& file)
{
  std::ifstream in(file.c_str());
  if (in) {
    std::string str((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    return str;
  }
  throw (errno);
}

GLuint loadShader(GLenum type, std::string const& s);
GLuint createProgram(std::string const& v, std::string const& f);
GLuint updateTexture2D(unsigned const texture_id, unsigned const& width, unsigned const& height,
    const char* data);
GLuint createTexture2D(unsigned const& width, unsigned const& height,
    const char* data);
GLuint createTexture3D(unsigned const& width, unsigned const& height,
    unsigned const& depth, int const internal_format, int const data_type,
    int const channel_format, const char* data);
#endif // #ifndef UTILS_HPP
