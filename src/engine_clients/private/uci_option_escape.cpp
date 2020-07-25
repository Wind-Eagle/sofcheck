#include "engine_clients/private/uci_option_escape.h"

#include <algorithm>
#include <cstring>

#include "util/strutil.h"

namespace SoFEngineClients::Private {

// Transforms each token in the string `name`. The given string must be a valid option name,
// otherwise the behaviour is undefined
template <typename Transform>
inline static std::string transformTokens(const std::string &name, Transform transform) {
  if (name.empty()) {
    return std::string();
  }
  std::string result;
  result.reserve(name.size() + 16);
  const char *str = name.c_str();
  str = SoFUtil::scanTokenStart(str);
  while (*str != '\0') {
    // Scan the next token
    const char *left = str;
    str = SoFUtil::scanTokenEnd(str);
    const char *right = str;
    str = SoFUtil::scanTokenStart(str);
    // Transform and add the token
    if (!result.empty()) {
      result += ' ';
    }
    result.append(transform(std::string(left, right)));
  }
  return result;
}

inline static bool isBadToken(const std::string &str) {
  // Find first characted not equal to "_"
  size_t pos = 0;
  while (pos < str.size() && str[pos] == '_') {
    ++pos;
  }
  for (const char *badWord : {"name", "value", "val"}) {
    if (std::equal(str.begin() + pos, str.end(), badWord, badWord + strlen(badWord))) {
      return true;
    }
  }
  return false;
}

inline static std::string uciNameEscape(const std::string &name) {
  return transformTokens(
      name, [&](const std::string &str) { return (isBadToken(str) ? "_" : "") + str; });
}

inline static std::string uciNameUnescape(const std::string &name) {
  return transformTokens(name, [&](const std::string &str) {
    return (isBadToken(str) && str[0] == '_') ? str.substr(1) : str;
  });
}

std::string uciOptionNameEscape(const std::string &name) { return uciNameEscape(name); }

std::string uciOptionNameUnescape(const std::string &name) { return uciNameUnescape(name); }

std::string uciEnumNameEscape(const std::string &item) { return uciNameEscape(item); }

std::string uciEnumNameUnescape(const std::string &item) { return uciNameUnescape(item); }

}  // namespace SoFEngineClients::Private
