#include "OCRText.h"


BCR_BEGIN_NAMESPACE

OCRText::OCRText()
{
	this->bold = false;
	this->italic = false;
	this->monspace = false;
	this->serif = false;
	this->smallcaps = false;
	this->fontSize = 0;
	this->fontId = 0;
	this->underline = false;
}


OCRText::OCRText(OCRText& txt)
{
	this->setPos(txt._pos);
	this->setSize(txt._size);
	this->setText(txt._text);

	this->bold = txt.bold;
	this->italic = txt.italic;
	this->monspace = txt.monspace;
	this->serif = txt.serif;
	this->smallcaps = txt.smallcaps;
	this->fontSize = txt.fontSize;
	this->fontId = txt.fontId;
	this->underline = txt.underline;
}


OCRText::~OCRText()
{
}


void OCRText::debugPrint()
{
	BCR_DEBUG_HEADER_LINE

	std::cout << _text << std::endl;
	std::cout << "\tBold:\t" << bold << std::endl;
	std::cout << "\tItalic:\t" << italic << std::endl;
	std::cout << "\tSize:\t" << fontSize << std::endl;
	std::cout << "\tFontId:\t" << fontId << std::endl;
}


cv::Point2i OCRText::pos()
{
	return _pos;
}


cv::Point2i OCRText::center()
{
	return cv::Point2i(_pos.x + _size.width / 2, _pos.y + _size.height / 2);
}


cv::Size OCRText::size()
{
	return _size;
}


std::string OCRText::text()
{
	return _text;
}


void OCRText::setPos(cv::Point2i pos)
{
	_pos = pos;
}


void OCRText::setSize(cv::Size size)
{
	_size = size;
}


void OCRText::setText(std::string text)
{
	_text = text;
}


int OCRText::top()
{
	return _pos.y;
}


int OCRText::middle()
{
	return (top() + bottom()) / 2;
}


int OCRText::bottom()
{
	return _pos.y + _size.height;
}


int OCRText::left()
{
	return _pos.x;
}


int OCRText::right()
{
	return _pos.x + _size.width;
}


int OCRText::numberOfWords()
{
	return explode_indices(this->_text, ' ').size();
}

BCR_END_NAMESPACE