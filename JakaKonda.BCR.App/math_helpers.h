#ifndef BCR_MATH_HELPERS_H
#define BCR_MATH_HELPERS_H

#include "bcr_common.h"


BCR_BEGIN_NAMESPACE

float rad2deg(float rad);

float deg2rad(float deg);

float slope(cv::Vec4i a);

float slope(cv::Point2f p1, cv::Point2f p2);

float slope(float p1x, float p1y, float p2x, float p2y);

float intersection_angle(cv::Vec4i a, cv::Vec4i b);

float intersection_angle(cv::Point2f a1, cv::Point2f a2, cv::Point2f b1, cv::Point2f b2);

float length(cv::Vec4i line);

float distance(cv::Point2f p1, cv::Point2f p2);

float distance(float x1, float y1, float x2, float y2);

cv::Point2f line_intersection(const cv::Vec4i a, const cv::Vec4i b);

cv::Point2f line_intersection(float x11, float y11, float x12, float y12, float x21, float y21, float x22, float y22);

BCR_END_NAMESPACE

#endif // #ifndef BCR_MATH_HELPERS_H