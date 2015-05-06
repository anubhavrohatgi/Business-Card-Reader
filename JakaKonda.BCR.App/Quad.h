#ifndef BCR_QUAD_H
#define BCR_QUAD_H

#include <numeric>
#include <vector>

#include "bcr_common.h"


BCR_BEGIN_NAMESPACE

class Quad
{
	public:
		Quad();

		float surface();
		Quad copy();

		void orderCorners();

		bool operator==(Quad &q);
		cv::Point2f operator[](int idx);

		std::vector<cv::Point2f> &points();

		cv::Point2f topRight();
		cv::Point2f topLeft();
		cv::Point2f bottomLeft();
		cv::Point2f bottomRight();

		float topLength();
		float bottomLength();
		float rightLength();
		float leftLength();

		bool isValid();
		void scale(float widthRatio, float heightRatio);

	private:
		std::vector<cv::Point2f> _points;
};

BCR_END_NAMESPACE

#endif // BCR_QUAD_H