#ifndef BCR_STRING_HELPERS_H
#define BCR_STRING_HELPERS_H

#include <string>

#include "bcr_common.h"


BCR_BEGIN_NAMESPACE

std::string to_lower(std::string s);

std::vector<std::string> explode(std::string const &s, char delim);

std::vector<int> explode_indices(std::string const &s, char delim);

std::string &trim(std::string &s);

std::string &ltrim(std::string &s); // trim from start

std::string &rtrim(std::string &s); // trim from end

std::string str_replace(std::string subject, const std::string &search, const std::string &replace);

void str_replace_in_place(std::string &subject, const std::string &search, const std::string &replace);

unsigned int levenshtein_distance(const std::string &s1, const std::string  &s2);

bool isparenthesis(char c);

int count_digits(std::string &str, int from, int len);

void str_delete(std::string &str, size_t from, size_t len, char repl);

bool is_vowel(char c);

bool starts_with(std::string haystack, std::string needle);

bool ends_with(std::string haystack, std::string needle);

BCR_END_NAMESPACE

#endif // BCR_STRING_HELPERS_H

