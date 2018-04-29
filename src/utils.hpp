#pragma once

#include <_types/_uint8_t.h>
#include <boost/iostreams/device/array.hpp>
#include <iosfwd>
#include <map>

namespace mtx {
namespace client {
namespace utils {

//! Generates a random string of the given size.
std::string
random_token(uint8_t len = 12, bool with_symbols = true) noexcept;

//! Construct query string from the given parameter pairs.
std::string
query_params(const std::map<std::string, std::string> &params) noexcept;

//! Decompress a response.
std::string
decompress(const boost::iostreams::array_source &src, const std::string &type) noexcept;
}
}
}
