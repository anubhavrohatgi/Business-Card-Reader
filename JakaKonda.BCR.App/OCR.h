#ifndef BCR_OCR_H
#define BCR_OCR_H

#include <iostream>
#include <string>
#include <memory>

#include <tesseract/baseapi.h>
#include <tesseract/strngs.h>

#include "OCRText.h"

#include "bcr_common.h"


BCR_BEGIN_NAMESPACE

class OCR
{
	public:
		OCR();
		
		void setImage(cv::Mat img);
		std::shared_ptr<OCRText> getOCRResult();

		bool isInitialized();

	private:
		bool init();
		void fix(std::string &str);

	private:
		tesseract::TessBaseAPI  _tessOcr;
		bool _init;
};

BCR_END_NAMESPACE

#endif