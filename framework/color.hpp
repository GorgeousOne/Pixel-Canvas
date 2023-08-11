#ifndef FENSTERCHEN_COLOR_HPP
#define FENSTERCHEN_COLOR_HPP

// -----------------------------------------------------------------------------
// Copyright  : (C) 2014 Andreas-C. Bernstein
// License    : MIT (see the file LICENSE)
// Maintainer : Andreas-C. Bernstein <andreas.bernstein@uni-weimar.de>
// Stability  : experimental
//
// Color
// -----------------------------------------------------------------------------

struct Color
{
  Color(float r, float g, float b) : r_(r), g_(g), b_(b) {}
  float r_;
  float g_;
  float b_;
};

#endif //#define FENSTERCHEN_COLOR_HPP
