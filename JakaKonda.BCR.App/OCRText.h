#ifndef BCR_OCR_TEXT_H
#define BCR_OCR_TEXT_H

#include <iostream>
#include <algorithm>

#include <opencv2/core/core.hpp>

#include "bcr_common.h"
#include "String.h"


BCR_BEGIN_NAMESPACE

struct OCRFontMeta
{
	bool bold;
	bool italic;
	bool monspace;
	bool serif;
	bool smallcaps;
	int fontSize;
	int fontId;
	bool underline;
};

class OCRText : public OCRFontMeta
{
	public:
		OCRText();
		OCRText(OCRText &txt);
		~OCRText();

		void debugPrint();

		cv::Point2i pos();
		cv::Point2i center();
		cv::Size size();
		std::string text();

		void setPos(cv::Point2i pos);
		void setSize(cv::Size size);
		void setText(std::string text);

		int top();
		int middle();
		int bottom();
		int left();
		int right();

		int numberOfWords();

	private:
		cv::Size _size;
		std::string _text;
		cv::Point2i _pos;
};

BCR_END_NAMESPACE

#endif // BCR_OCR_TEXT_H