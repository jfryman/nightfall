#ifndef NIGHTFALL_CORE_RESOURCEFORK_H
#define NIGHTFALL_CORE_RESOURCEFORK_H

#include "nightfall.h"

#include <stdint.h>

#include <string>
#include <vector>

namespace nightfall::resource {

struct ResourceEntry {
  uint32_t type;
  int16_t id;
  uint8_t attributes;
  std::string name;
  std::vector<uint8_t> data;
};

struct ResourceFork {
  uint16_t file_attributes;
  std::vector<ResourceEntry> entries;
};

uint32_t make_type_code(char a, char b, char c, char d);
std::string type_code_to_string(uint32_t type);
nf_status parse_resource_fork(const uint8_t *bytes, size_t byte_count, ResourceFork *out_fork);
const ResourceEntry *find_resource(const ResourceFork &fork, uint32_t type, int16_t id);
size_t count_type(const ResourceFork &fork, uint32_t type);

}  // namespace nightfall::resource

#endif
