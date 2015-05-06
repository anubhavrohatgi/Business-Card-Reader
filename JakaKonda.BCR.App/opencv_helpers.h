#ifndef BCR_OPENCV_HELPERS_H
#define BCR_OPENCV_HELPERS_H

#include <opencv2/contrib/contrib.hpp>

#include "bcr_common.h"


BCR_BEGIN_NAMESPACE

// Calculates median for single channel image
cv::Scalar mat_median(cv::Mat image);

cv::Mat auto_canny(cv::Mat img);

cv::Mat maximize_contrast(cv::Mat img);

cv::Mat resize(cv::Mat img, int maxWidth, int maxHeight);

cv::Mat to_gray(cv::Mat src);

int to_bin(const float angle, const int neighbors = 8);

int count_neighbours(cv::Mat img, int i, int j);

cv::Mat MSER_filter(cv::Mat input);

BCR_END_NAMESPACE

#endif // BCR_OPENCV_HELPERS_H