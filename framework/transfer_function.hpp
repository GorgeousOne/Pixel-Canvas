#ifndef TRANSFER_FUNCTION_HPP
#define TRANSFER_FUNCTION_HPP

#include "data_types_fwd.hpp"

#include <string>
#include <map>

#define GLM_FORCE_RADIANS
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <utils.hpp>
#include <plane.hpp>

class Transfer_function
{
public:
  typedef std::pair<unsigned, glm::vec4> element_type;
  typedef std::map<unsigned, glm::vec4>  container_type;

public:
  Transfer_function();
  ~Transfer_function() {}

  void add(float, glm::vec4);
  void add(unsigned, glm::vec4);
    
  void remove(unsigned);

  void reset();

  image_data_type          get_RGBA_transfer_function_buffer() const;
  //void                  update_and_draw();
  void                  draw_texture(glm::vec2 const& window_dim, glm::vec2 const& tf_pos, GLuint const& texture) const;
  container_type&       get_piecewise_container(){ return m_piecewise_container;};

private:
    //void update_vbo();

private:
  container_type    m_piecewise_container;
  
  unsigned int      m_program_id;
  //unsigned int      m_vao;
  Plane             m_plane;
    
  bool              m_dirty;
};

#endif // define TRANSFER_FUNCTION_HPP
