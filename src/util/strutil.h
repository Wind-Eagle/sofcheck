#ifndef SOF_UTIL_STRUTIL_INCLUDED
#define SOF_UTIL_STRUTIL_INCLUDED

#include <charconv>
#include <cstring>
#include <string>

namespace SoFUtil {

// Scans the string forward, starting from `str` and returns the first character which is considered
// the end of the token (i.e. one of the characters `'\0'`, `'\t'`, `'\n'` and `' '`).
const char *scanTokenEnd(const char *str);

// Scans the string forward, starting from `str` and returns the first character which is not equal
// to `'t'`, `'\n'` and `' '`.
const char *scanTokenStart(const char *str);

// Converts the character `c` to lower case. `c` must be an ASCII character
inline constexpr char asciiToLower(const char c) {
  return ('A' <= c && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

// Converts the character `c` to upper case. `c` must be an ASCII character
inline constexpr char asciiToUpper(const char c) {
  return ('a' <= c && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
}

// Wrapper for `std::from_chars`. Tries to interpret the entire string between `first` and `last` as
// integer or floating point type `T`. Returns `true` and puts the result into `val` on success.
// Otherwise `false` is returned and `val` remains untouched.
template <typename T>
inline bool valueFromStr(const char *first, const char *last, T &val) {
  T value;
  const std::from_chars_result res = std::from_chars(first, last, value);
  if (res.ptr != last || res.ec != std::errc()) {
    return false;
  }
  val = value;
  return true;
}

// Replaces the small characters (with ASCII code <= 32) with spaces. The tab characters ('\t') are
// left intact. The primary purpose of this function is to sanitize the string for UCI output, to
// make sure that it doesn't contain a newline, which can be potentially interpreted as a new UCI
// command
std::string sanitizeEol(std::string str);

// Removes leading and trailing spaces from the string
std::string trim(const std::string &str);

// Works like `strcpy`, but returns the address of null-terminating byte in `dst` (e.g. pointer to
// the end of `dst` instead of the pointer to the beginning of `dst`)
//
// For more details, see https://www.man7.org/linux/man-pages/man3/stpcpy.3.html
#ifdef USE_SYSTEM_STPCPY
inline char *stpcpy(char *dst, const char *src) { return ::stpcpy(dst, src); }
#else
inline char *stpcpy(char *dst, const char *src) {
  strcpy(dst, src);  // NOLINT
  return dst + strlen(src);
}
#endif

}  // namespace SoFUtil

#endif  // SOF_UTIL_STRUTIL_INCLUDED
