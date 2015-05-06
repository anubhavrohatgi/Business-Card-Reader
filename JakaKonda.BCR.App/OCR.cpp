#include "OCR.h"


BCR_BEGIN_NAMESPACE

OCR::OCR()
{
	_init = init();
}


bool OCR::init()
{
	if(_tessOcr.Init(nullptr, "eng", tesseract::OEM_DEFAULT) == -1)
		return false;

	_tessOcr.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
	_tessOcr.SetVariable("tessedit_char_whitelist",	"-0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@+().,:ß/&");
	
	return true;
}


void OCR::setImage(cv::Mat img)
{
	_tessOcr.SetImage(static_cast<uchar *>(img.data), 
					  img.cols, img.rows, 1, img.cols);
}


std::shared_ptr<OCRText> OCR::getOCRResult()
{
	auto result = std::make_shared<OCRText>();

	_tessOcr.Recognize(nullptr);
	auto ri = _tessOcr.GetIterator();

	//
	char *data = ri->GetUTF8Text(tesseract::RIL_BLOCK);

	if (data == nullptr)
		return result;

	std::string str = data;
	str = trim(str);

	if (str.empty())
		return result;

	ri->WordFontAttributes(
		&result->bold,
		&result->italic,
		&result->underline,
		&result->monspace,
		&result->serif,
		&result->smallcaps,
		&result->fontSize,
		&result->fontId);

	// Round to bottom even number, to decrease mistakes in size
	result->fontSize -= result->fontSize % 2;

	fix(str);
	result->setText(str);

	// Clear cached data
	_tessOcr.Clear();
	str.clear();

	return result;
}


void OCR::fix(std::string &str)
{	
	str = trim(str);

	// Fix most common OCR mistakes occurring in Business Cards
	str_replace_in_place(str, ")(", "X");
	str_replace_in_place(str, "\n\n", "\n");
	str_replace_in_place(str, ". ", ".");
	str_replace_in_place(str, "www", " www");
	str_replace_in_place(str, " l ", " / ");
	str_replace_in_place(str, "F ax", "Fax");
	str_replace_in_place(str, "@ ", "@");
	str_replace_in_place(str, " @", "@");
	str_replace_in_place(str, "  ", " ");

	// Fix characters in number areas or vice versa
	bool prevNr = false;
	bool prevChar = false;
	bool prevSpace = false;
	bool prevLower = false;

	for (int i = 0; i < str.length(); i++)
	{
		if (isspace(str[i]))
		{
			prevSpace = true;
			continue;
		}

		if (!isalnum(str[i]))
			continue;

		bool isNextNr = false;

		if (i + 1 < str.length() && isdigit(str[i + 1]))
			isNextNr = true;

		
		if (!prevSpace)
		{ 
			if (prevNr)
			{
				switch (str[i])
				{
					case 'l':
						str[i] = '1';
						break;

					case '&':
						str[i] = '8';
						break;

					case 'S':
						if (isNextNr)
							str[i] = '5';
						break;

					case 'o':
					case 'O':
						str[i] = '0';
						break;
					case 'M':
						str[i] = '/';
						str.insert(str.begin() + i + 1, '4');
					case 'I':
						if (i + 2 < str.size() && (str[i + 2] == 'I' || str[i + 2] == '1') && isdigit(str[i + 1]))
						{
							str[i] = '(';
							str[i + 2] = ')';
						}
				}
			}
			else if (prevChar)
			{
				switch (str[i])
				{
					case 'I':
						if (prevLower)
							str[i] = 'l';
						break;
					case '6':
						if (!prevLower)
							str[i] = 'G';
					
					// TODO: Unicode support
					//case 'B':
					//	if (prevLower)
					//		str[i] = 'ß';
					//	break;
				}
			}
		}
		else
		{
			if (prevNr && isNextNr)
			{
				switch (str[i])
				{
					case 'l':
						str[i] = '1';
				}
			}
			if (prevChar && i + 1 < str.size() && (str[i + 1] == ':' || str[i+1] == 't') && str[i] == '8')
			{ 
				str[i] = '&';
				str.erase(str.begin() + i+1);
			}

			if (i + 1 < str.size() && str[i + 1] == ' ' && str[i] == 'o')
			{
				str[i] = ',';
			}
			if (i - 1 > 0 && str[i-1] == '.' && isalpha(str[i]))
				str[i-1] = ',';
		}
		if (i - 1 >= 0 && !isdigit(str[i - 1]) && str[i] == '0' && i + 1< str.size() && isalpha(str[i + 1]))
			str[i] = 'O';

		prevSpace = false;
		prevNr = isdigit(str[i]) != 0;
		prevChar = isalpha(str[i]) != 0;
		if (prevChar)
			prevLower = islower(str[i]);
	}

	str = trim(str);
}


bool OCR::isInitialized()
{
	return _init;
}

BCR_END_NAMESPACE