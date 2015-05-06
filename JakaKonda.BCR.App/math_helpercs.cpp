#include "math_helpers.h"


BCR_BEGIN_NAMESPACE

float rad2deg(float rad)
{
	return rad * (180 / CV_PI);
}


float deg2rad(float deg)
{
	return deg * (CV_PI / 180);
}


float slope(cv::Vec4i a)
{
	return slope(a[0], a[1], a[2], a[3]);
}


float slope(cv::Point2f p1, cv::Point2f p2)
{
	return slope(p1.x, p1.y, p2.x, p2.y);
}


float slope(float p1x, float p1y, float p2x, float p2y)
{
	if (p1x == p2x)
		return BCR_FLT_MAX;

	return (p1y - p2y) / (p1x - p2x);
}


float intersection_angle(cv::Vec4i a, cv::Vec4i b)
{
	float slopeA = slope(a);
	float slopeB = slope(b);

	return atan(std::max(slopeA, slopeB)) - atan(std::min(slopeA, slopeB));
}


float intersection_angle(cv::Point2f a1, cv::Point2f a2, cv::Point2f b1, cv::Point2f b2)
{
	auto angle = abs(atan2(a2.y - a1.y, a2.x - a1.x) - atan2(b2.y - b1.y, b2.x - b1.x));

	if (angle > CV_PI)
		angle = 2 * CV_PI - angle;

	return angle;
}


float length(cv::Vec4i l)
{
	return distance(l[0], l[1], l[2], l[3]);
}


float distance(cv::Point2f p1, cv::Point2f p2)
{
	return distance(p1.x, p1.y, p2.x, p2.y);
}


float distance(float x1, float y1, float x2, float y2)
{
	return sqrt(SQUARED(x1 - x2) + SQUARED(y1 - y2));
}


cv::Point2f line_intersection(const cv::Vec4i a, const cv::Vec4i b)
{
	return line_intersection(a[0], a[1], a[2], a[3], b[0], b[1], b[2], b[3]);
}


cv::Point2f line_intersection(float x11, float y11, float x12, float y12,
	float x21, float y21, float x22, float y22)
{
	//https://en.wikipedia.org/wiki/Line–line_intersection
	if (float d = ((x11 - x12) * (y21 - y22) - (y11 - y12) * (x21 - x22)))
	{
		cv::Point2f pt;
		pt.x = ((x11*y12 - y11*x12) * (x21 - x22) - (x11 - x12) * (x21*y22 - y21*x22)) / d;
		pt.y = ((x11*y12 - y11*x12) * (y21 - y22) - (y11 - y12) * (x21*y22 - y21*x22)) / d;

		return pt;
	}
	else
		return cv::Point2f(-1, -1);
}



BCR_END_NAMESPACE