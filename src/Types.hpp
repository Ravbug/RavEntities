#pragma once
#include <numeric>

using entity_id_t = uint32_t;
using pos_t = uint32_t;
static constexpr pos_t INVALID_INDEX = std::numeric_limits<decltype(INVALID_INDEX)>::max();
