#include "TextGroup.h"


BCR_BEGIN_NAMESPACE

float TextGroup::distance(TextGroup &g)
{
	float minDist = FLT_MAX;

	for (size_t i = 0; i < this->size(); i++)
	{
		for (size_t j = 0; j < g.size(); j++)
		{
			// % of overlapping on X axis
			float overlapingDist =
				min(at(i).get()->right(), g[j].get()->right()) -
				max(at(i).get()->left(), g[j].get()->left());

			if (overlapingDist < 0)
				continue;

			auto bestOverlapingRect =
				max(overlapingDist*1.0 / at(i).get()->size().width*1.0,
				overlapingDist*1.0 / g[j].get()->size().width*1.0);

			if (bestOverlapingRect < 0.3)
				continue;

			// Vertical space between text boxes
			auto verticalSpace =
				min(at(i)->bottom(), g[j].get()->bottom()) -
				max(at(i)->top(), g[j].get()->top());

			// Rejection criteria:
			//  - If it's smaller than means that are next to each other
			//  - If it's greater than means that area to far apart 
			//if ((verticalSpace < -at(i).get()->size().height * 0.5 &&
			//	verticalSpace < -g[j].get()->size().height * 0.5)
			//	||
			//	(verticalSpace > at(i).get()->size().height * 0.75 &&
			//	verticalSpace >  g[j].get()->size().height * 0.75))
			//	continue;

			if (abs(verticalSpace) > 14 && abs(at(i)->top() - g[j]->top()) >  6)
				continue;

			minDist = abs(verticalSpace * (1 - bestOverlapingRect + 0.1));
		}
	}

	return minDist == FLT_MAX ? -1 : minDist;
}


void TextGroup::merge(TextGroup &g)
{
	this->reserve(this->size() + g.size());
	this->insert(this->end(), g.begin(), g.end());
}


void TextGroup::debugPrint()
{
	BCR_DEBUG_HEADER_LINE

	for (auto i = begin(); i != end(); ++i)
		cout << (*i).get()->text() << endl;
}


void TextGroup::sort()
{
	std::sort(this->begin(), this->end(),
		[](const shared_ptr<OCRText> &a, const shared_ptr<OCRText> &b) -> bool
	{
		int top = a.get()->top() - b.get()->top();

		if (top != 0)
			return top < 0;

		return a.get()->left() < b.get()->left();
	});
}


bool TextGroup::contains(shared_ptr<OCRText> ocrTxt)
{
	return position(ocrTxt) != INT_MAX;
}


int TextGroup::position(shared_ptr<OCRText> ocrTxt)
{
	for (size_t i = 0; i < this->size(); i++)
		if (this->at(i) == ocrTxt)
			return i;

	return INT_MAX;
}

BCR_END_NAMESPACE
