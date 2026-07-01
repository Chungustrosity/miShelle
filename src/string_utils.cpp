#include "string_utils.hpp"
#include <cctype>

size_t skipWhitespace(const std::string &str, size_t startIndex) {
  size_t i = startIndex;
  while (i < str.size() && std::isspace(str[i]))
    i++;
  return i;
}

strviewVector splitOnSpace(const std::string &fullText) {
  strviewVector words;
  std::string_view fullText_view(fullText);
  size_t l, r;
  l = r = skipWhitespace(fullText);
  while (l < fullText_view.size()) {
    while (r < fullText_view.size() && !std::isspace(fullText_view[r]))
      r++;

    std::string_view word = fullText_view.substr(l, r - l);
    words.push_back(word);

    l = r = skipWhitespace(fullText, r);
  }
  return words;
}

std::string expandToken(const std::string &str) {
  std::string finalToken;
  for (int i; i < str.size(); i++) {
    if (str[i] == '~') {
      if (i == 0 || str[i - 1] == ' ')
        finalToken += std::getenv("HOME");
    } else
      finalToken += str[i];
  }
  return str;
}
