#include "volume_loader_raw.hpp"

#include <iostream>
#include <fstream>

volume_data_type
Volume_loader_raw::load_volume(std::string filepath)
{
  std::ifstream volume_file;
  volume_file.open(filepath, std::ios::in | std::ios::binary);

  volume_data_type data;

  if (volume_file.is_open()) {
    glm::ivec3 vol_dim = get_dimensions(filepath);
    unsigned channels = get_channel_count(filepath);
    unsigned byte_per_channel = get_bit_per_channel(filepath) / 8;


    size_t data_size = vol_dim.x
                      * vol_dim.y
                      * vol_dim.z
                      * channels
                      * byte_per_channel;

    
    data.resize(data_size);

    volume_file.seekg(0, std::ios::beg);
    volume_file.read((char*)&data.front(), data_size);
    volume_file.close();

    //std::cout << "File " << filepath << " successfully loaded" << std::endl;

    return data;
  } else {
    std::cerr << "File " << filepath << " doesnt exist! Check Filepath!" << std::endl;
    assert(0);
    return data;
  }

  //never reached
  assert(0);
  return data;
}

glm::ivec3 Volume_loader_raw::get_dimensions(const std::string filepath) const
{
  unsigned width = 0;
  unsigned height = 0;
  unsigned depth = 0;

  //"name_wxx_hxx_dxx_cx_bx"
  size_t p0 = 0, p1 = std::string::npos;

  p0 = filepath.find("_w", 0) + 2;
  p1 = filepath.find("_h", 0);

  if (p1 != p0) {
    std::string token = filepath.substr(p0, p1 - p0);
    width = std::atoi(token.c_str());
  }

  p0 = filepath.find("_h", p0) + 2;
  p1 = filepath.find("_d", p0);

  if (p1 != p0) {
    std::string token = filepath.substr(p0, p1 - p0);
    height = std::atoi(token.c_str());
  }

  p0 = filepath.find("_d", p0) + 2;
  p1 = filepath.find("_c", p0);

  if (p1 != p0) {
    std::string token = filepath.substr(p0, p1 - p0);
    depth = std::atoi(token.c_str());
  }

  return glm::ivec3(width, height, depth);
}

unsigned Volume_loader_raw::get_channel_count(const std::string filepath) const
{
  unsigned channels = 0;

  size_t p0 = 0, p1 = std::string::npos;

  p0 = filepath.find("_c", 0) + 2;
  p1 = filepath.find("_b", p0);

  std::string token = filepath.substr(p0, p1 - p0);
  channels = std::atoi(token.c_str());

  return channels;
}

unsigned Volume_loader_raw::get_bit_per_channel(const std::string filepath) const
{
  unsigned byte_per_channel = 0;

  size_t p0 = 0, p1 = std::string::npos;

  p0 = filepath.find("_b", 0) + 2;
  p1 = filepath.find(".", p0);

  std::string token = filepath.substr(p0, p1 - p0);
  byte_per_channel = std::atoi(token.c_str());

  return byte_per_channel;
}
