#ifndef BCR_BCR_TEXT_CLASSIFIER_H
#define BCR_BCR_TEXT_CLASSIFIER_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <regex>

#include "StringMatcher.h"
#include "BusinessCard.h"
#include "TextGroup.h"
#include "OCRText.h"

#include "bcr_common.h"


BCR_BEGIN_NAMESPACE

class BCRTextClassifier
{
	typedef shared_ptr<OCRText> PtrOCRText;

	// Sort function ptr, for ordering results
	static const std::function<int(PtrOCRText, PtrOCRText)> _sortFunc;

	struct NumberArea
	{
		shared_ptr<OCRText> text_ptr;
		int text_index;
		int startIndex;
		int length;
	};

	public:
		BCRTextClassifier();
		~BCRTextClassifier();

		void process();
		BusinessCard &bussinesCard();

		void setText(vector<shared_ptr<OCRText>> strings);

	private:
		bool isJunk(std::string &str);
		void preprocessText();
		void groupText();
		void findNumberAreas();
		bool removeText(shared_ptr<OCRText> &search);
		void clear();
		int matchPhoneNumber(std::string &search);

		void findEmail();
		void findWebSite();
		void findAddress();
		void findPhoneNumbers();
		void findFirstLastName();
		void findCompany();

		static bool isGoodName(std::string str);

	private:
		BusinessCard _bc;

		std::vector<std::shared_ptr<OCRText>> _text;
		std::vector<TextGroup> _textGroups;
		std::vector<NumberArea> _numbers;
		std::vector<NumberArea> _addresses;
};

BCR_END_NAMESPACE

#endif // BCR_BCR_TEXT_CLASSIFIER_H