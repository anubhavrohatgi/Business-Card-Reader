#include "string_helpers.h"


BCR_BEGIN_NAMESPACE

std::string to_lower(std::string s)
{
	std::string str = s;
	
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);

	return str;
}


std::vector<std::string> explode(std::string const &s, char delim)
{
	std::vector<std::string> result;
	std::istringstream iss(s);

	for (std::string token; std::getline(iss, token, delim);)
		result.push_back(std::move(token));

	return result;
}


std::vector<int> explode_indices(std::string const &s, char delim)
{
	std::vector<int> result;

	bool found = false;

	for (size_t i = 0; i < s.size(); i++)
		if (s[i] != delim)
		{
			if (!found)
				result.push_back(i);

			found = true;
		}
		else
			found = false;

	return result;
}


unsigned int levenshtein_distance(const std::string &s1, const std::string &s2)
{
	const std::size_t len1 = s1.size(), len2 = s2.size();
	std::vector<unsigned int> col(len2 + 1), prevCol(len2 + 1);

	for (unsigned int i = 0; i < prevCol.size(); i++)
		prevCol[i] = i;

	for (unsigned int i = 0; i < len1; i++) 
	{
		col[0] = i + 1;
		for (unsigned int j = 0; j < len2; j++)
			col[j + 1] = std::min({ prevCol[1 + j] + 1, col[j] + 1, prevCol[j] + (s1[i] == s2[j] ? 0 : 1) });
		col.swap(prevCol);
	}

	return prevCol[len2];
}


bool isparenthesis(char c)
{
	return c == '(' || c == ')';
}


std::string &trim(std::string &s) {
	return ltrim(rtrim(s));
}


std::string &ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}


std::string &rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}


std::string str_replace(std::string subject, 
	const std::string &search,
	const std::string &replace)
{
	size_t pos = 0;

	while ((pos = subject.find(search, pos)) != std::string::npos) 
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}

	return subject;
}


void str_replace_in_place(std::string &subject, 
	const std::string &search,
	const std::string &replace)
{
	size_t pos = 0;

	while ((pos = subject.find(search, pos)) != std::string::npos) 
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}


int count_digits(std::string &str, int from, int len)
{
	int nr = 0;

	for (int i = from; i < from + len && i < str.size(); i++)
		if (isdigit(str[i]))
			nr++;

	return nr;
}


void str_delete(std::string &str, size_t from, size_t len, char repl)
{
	for (size_t i = from; i < str.length() && i < from + len; i++)
		str[i] = repl;
}


bool is_vowel(char c)
{
	switch (tolower(c))
	{
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
			return true;
	}

	return false;
}


bool starts_with(std::string haystack, std::string needle)
{
	if (haystack.size() < needle.size())
		return false;

	return !haystack.compare(0, needle.size(), needle);
}

bool ends_with(std::string fullString, std::string ending)
{
	if (fullString.length() >= ending.length())
		return 0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending);

	return false;
}


BCR_END_NAMESPACE