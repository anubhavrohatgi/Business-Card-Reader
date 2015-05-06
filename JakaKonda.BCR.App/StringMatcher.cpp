#include "StringMatcher.h"

using namespace std;


BCR_BEGIN_NAMESPACE

StringMatcher::StringMatcher(vector<shared_ptr<OCRText>> &strings)
{
	_text = strings;
}


StringMatcher::~StringMatcher()
{
}


// Use running window, where an item is a single word
// Find best window using Levenshtein distance
void StringMatcher::match(std::string find)
{
	_bestMatchIdx = 0;
	_bestMatchCost = INT_MAX;
	_bestMatchStart = 0;
	_bestMatchEnd = 0;

	// Put everything to lower for better matching with urls
	find = to_lower(find);

	for (int i = 0; i < _text.size(); i++)
	{
		auto text = to_lower(_text[i]->text());
		auto words = explode_indices(text, ' ');

		vector<int> runningWnd;
		runningWnd.reserve(4);

		bool isBetter = true;
		int prev = INT_MAX;

		while (runningWnd.size() + words.size() > 0)
		{
			// If result in current string is better than previous one
			// add another string, otherwise delete (first) one
			if (isBetter || runningWnd.size() == 0)
			{ 
				if (words.size() > 0)
				{
					runningWnd.push_back(words[0]);
					words.erase(words.begin());
				}
				else
				{
					runningWnd.erase(runningWnd.begin());

					if (runningWnd.size() == 0)
						break;
				}
			}
			
			int next = words.size() > 0 ? words[0] : text.length() - 1;

			auto s = string(text.begin() + runningWnd.front(),
							text.begin() + next);
			s = trim(s);
			int curr = 0;

			// Longer word should be on top for better result
			if (s.size() > find.size())
				curr = levenshtein_distance(s, find);
			else
				curr = levenshtein_distance(find, s);

			curr = curr;
			isBetter = curr < prev;

			if (curr < _bestMatchCost)
			{
				_bestMatchCost = curr;
				_bestMatchIdx = i;
				_bestMatchStart = runningWnd.front();
				_bestMatchEnd = words.size() > 0 ? words[0] : text.length() - 1;
			}

			// These are extremly good results, save them
			// varry only 10% => 1 wrong character out of 10
			if (curr <= ceil(find.size()* 0.1))
				_results.push_back(_text[i]);

			if (!isBetter)
				runningWnd.erase(runningWnd.begin());

			prev = curr;
		}
	}

	// Remove duplicates from result set.
	// Unique requires sorted container, but it's already inserted
	// in ascending order
	_results.erase(unique(_results.begin(), _results.end()), _results.end());
}


int StringMatcher::getBestMatchIndex()
{
	return _bestMatchIdx;
}


int StringMatcher::getBestMatchCost()
{
	return _bestMatchCost;
}


int StringMatcher::getBestMatchStart()
{
	return _bestMatchStart;
}


int StringMatcher::getBestMatchEnd()
{
	return _bestMatchEnd;
}


int StringMatcher::getBestMatchLength()
{
	return _bestMatchEnd - _bestMatchStart;
}


std::vector<shared_ptr<OCRText>> &StringMatcher::searchData()
{
	return _text;
}


std::vector<std::shared_ptr<OCRText>>& StringMatcher::results()
{
	return _results;
}

BCR_END_NAMESPACE