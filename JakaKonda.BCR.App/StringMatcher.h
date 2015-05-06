#ifndef BCR_STRING_MATCHER_H
#define BCR_STRING_MATCHER_H

#include <iostream>
#include <memory>
#include <vector>

#include "OCRText.h"

#include "bcr_common.h"


BCR_BEGIN_NAMESPACE

class StringMatcher
{
	public:
		StringMatcher(std::vector<std::shared_ptr<OCRText>> &strings);
		~StringMatcher();

		void match(std::string find);

		int getBestMatchIndex();
		int getBestMatchCost();
		int getBestMatchStart();
		int getBestMatchEnd();
		int getBestMatchLength();

		std::vector<std::shared_ptr<OCRText>> &searchData();
		std::vector<std::shared_ptr<OCRText>> &results();

	private:
		int _bestMatchCost;
		int _bestMatchIdx;
		int _bestMatchStart;
		int _bestMatchEnd;

		std::vector<std::shared_ptr<OCRText>> _text;
		std::vector<std::shared_ptr<OCRText>> _results;
};

BCR_END_NAMESPACE

#endif // BCR_BCR_STRING_MATCHER_H

