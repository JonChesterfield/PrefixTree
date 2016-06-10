#include "prefix.hpp"

namespace
{
struct minimal : prefix::crtp<minimal>
{
  static constexpr element table[] = {
      "death", "life",
  };
};

constexpr decltype(minimal::table) minimal::table;
}
extern "C" std::size_t life(const char* key)
{
  return minimal::lookup_index(key);
}
