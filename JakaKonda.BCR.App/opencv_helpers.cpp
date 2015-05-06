#include "opencv_helpers.h"


BCR_BEGIN_NAMESPACE

cv::Scalar mat_median(cv::Mat image)
{
	double m = image.rows*image.cols / 2;
	int bin0 = 0;

	cv::Scalar med;
		med.val[0] = -1;

	int histSize = 256;
	float range [] = { 0, 256 };

	const float* histRange = { range };

	bool uniform = true;
	bool accumulate = false;

	cv::Mat hist;
	std::vector<cv::Mat> channels;

	cv::split(image, channels);
	cv::calcHist(&channels[0], 1, nullptr, cv::Mat(), hist, 
				 1, &histSize, &histRange, uniform, accumulate);

	for (int i = 0; i<256 && (med.val[0]<0 || med.val[1]<0 || med.val[2]<0); i++)
	{
		bin0 = bin0 + cvRound(hist.at<float>(i));

		if (bin0>m && med.val[0]<0)
			med.val[0] = i;
	}

	return med;
}


cv::Mat auto_canny(cv::Mat img)
{
	const double sigma = 0.33;

	auto imgMedian = mat_median(img).val[0];

	int lower = std::max(0.0, (1.0 - sigma) * imgMedian);
	int upper = std::min(255.0, (1.0 - sigma) * imgMedian);

	cv::Mat ret;
	cv::Canny(img, ret, lower + 10, upper + 20);

	return ret;
}


cv::Mat maximize_contrast(cv::Mat img)
{
	double minVal, maxVal;
	cv::minMaxLoc(img, &minVal, &maxVal);

	cv::Mat res;
	img.convertTo(res, CV_8UC1, 255.0 / (maxVal - minVal), 
				  -minVal * 255.0 / (maxVal - minVal));

	return res;
}


cv::Mat resize(cv::Mat img, int maxWidth, int maxHeight)
{
	int width = img.cols;
	int height = img.rows;


	float ratio = maxWidth*1. / width;
	width *= ratio;
	height *= ratio;

	if (height > maxHeight)
	{
		ratio = maxHeight*1. / height;

		width *= ratio;
		height *= ratio;
	}

	cv::resize(img, img, cv::Size(width, height));

	return img;
}


cv::Mat to_gray(cv::Mat src)
{
	cv::Mat gray;
	cv::cvtColor(src, gray, CV_BGR2GRAY, CV_8UC1);
	return gray;
}


int to_bin(const float angle, const int neighbors)
{
	const float divisor = 180.0 / neighbors;
	return static_cast<int>((floor(angle / divisor) - 1) / 2 + 1) % neighbors + 1;
}


int count_neighbours(cv::Mat img, int i, int j)
{
	int count = 0;

	if (i - 1 > 0 && img.at<uchar>(i - 1, j) > 0)
		count++;

	if (i + 1 < img.rows && img.at<uchar>(i+1, j) > 0)
		count++;

	if (j - 1 > 0 && img.at<uchar>(i, j - 1) > 0)
		count++;

	if (j + 1 < img.cols && img.at<uchar>(i, j + 1) > 0)
		count++;

	return count;
}


cv::Mat MSER_filter(cv::Mat input)
{
	cv::Mat gray;
	
	if (input.channels() > 1)
		gray = to_gray(input);
	else
		gray = input.clone();

	std::vector<std::vector<cv::Point2i>> contours;
	cv::MSER mser(8, 10, 2000, 0.25, 0.1, 100, 1.01, 0.03, 5);
	mser(gray, contours);

	/* Create a binary mask out of the MSER */
	cv::Mat mser_mask(gray.size(), CV_8UC1, cv::Scalar(0));

	for (int i = 0; i < contours.size(); i++) {
		for (auto point : contours[i])
			mser_mask.at<uchar>(point) = 255;
	}

	return mser_mask;
}

BCR_END_NAMESPACE