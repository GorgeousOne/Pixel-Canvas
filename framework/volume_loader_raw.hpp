#ifndef VOLUME_LOADER_RAW_HPP
#define VOLUME_LOADER_RAW_HPP

#include "data_types_fwd.hpp"

#include <array>
#include <string>

#include <glm/vec3.hpp>


class Volume_loader_raw
{
public:
  Volume_loader_raw() {}

  volume_data_type load_volume(std::string file_path);

  glm::ivec3 get_dimensions(const std::string file_path) const;
  unsigned   get_channel_count(const std::string file_path) const;
  unsigned   get_bit_per_channel(const std::string file_path) const;
private:
};

#endif // define VOLUME_LOADER_RAW_HPP
