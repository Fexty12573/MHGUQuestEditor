#pragma once

#include <cstdlib>
#include <cstdint>

#include <type_traits>

namespace detail {
namespace crc {

template <unsigned c, int k = 8> struct f : f<((c & 1) ? 0xedb88320 : 0) ^ (c >> 1), k - 1> {};
template <unsigned c> struct f<c, 0> {
    enum { value = c };
};

#define CRC32_INIT_A(x) CRC32_INIT_B(x) CRC32_INIT_B(x + 128)
#define CRC32_INIT_B(x) CRC32_INIT_C(x) CRC32_INIT_C(x + 64)
#define CRC32_INIT_C(x) CRC32_INIT_D(x) CRC32_INIT_D(x + 32)
#define CRC32_INIT_D(x) CRC32_INIT_E(x) CRC32_INIT_E(x + 16)
#define CRC32_INIT_E(x) CRC32_INIT_F(x) CRC32_INIT_F(x + 8)
#define CRC32_INIT_F(x) CRC32_INIT_G(x) CRC32_INIT_G(x + 4)
#define CRC32_INIT_G(x) CRC32_INIT_H(x) CRC32_INIT_H(x + 2)
#define CRC32_INIT_H(x) CRC32_INIT_I(x) CRC32_INIT_I(x + 1)
#define CRC32_INIT_I(x) f<x>::value,

constexpr unsigned crc_table[] = { CRC32_INIT_A(0) };

// Constexpr implementation and helpers
constexpr uint32_t crc32_impl(const char* p, size_t len, uint32_t crc) {
    return len ? crc32_impl(p + 1, len - 1, (crc >> 8) ^ crc_table[(crc & 0xFF) ^ *p]) : crc;
}

template <class T, typename = std::enable_if_t<sizeof(T) == 1>>
constexpr uint32_t crc32(const T* data, size_t length) {
    return crc32_impl(data, length, ~0);
}

}
}

constexpr inline uint32_t operator""_ext(const char* s, size_t len) {
    return detail::crc::crc32(s, len) & 0x7FFFFFFF;
}

constexpr inline uint32_t operator""_crc(const char* s, size_t len) {
    return detail::crc::crc32(s, len) & 0x7FFFFFFF;
}

constexpr inline uint32_t type_hash(const char* s) {
    return detail::crc::crc32(s, std::char_traits<char>::length(s)) & 0x7FFFFFFF;
}
