#ifndef BCR_TEXT_GROUP_H
#define BCR_TEXT_GROUP_H

#include <iostream>
#include <memory>
#include <vector>
#include <limits>

#include "OCRText.h"

#include "bcr_common.h"

using namespace std;


BCR_BEGIN_NAMESPACE

class TextGroup : public std::vector<std::shared_ptr<OCRText>>
{
	public:
	float distance(TextGroup &g);
	void merge(TextGroup &g);

	void sort();
	bool contains(shared_ptr<OCRText> ocrTxt);
	int position(shared_ptr<OCRText> ocrTxt);

	void debugPrint();
};

BCR_END_NAMESPACE

#endif // BCR_TEXT_GROUP_H