#ifndef BCR_BCR_READER_H
#define BCR_BCR_READER_H

#include <iostream>
#include <vector>
#include <memory>
#include <list>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "OCR.h"

#include "BCRTextClassifier.h"
#include "StringMatcher.h"
#include "BusinessCard.h"
#include "TextGroup.h"
#include "OCRText.h"
#include "String.h"
#include "Quad.h"


#include "bcr_common.h"


BCR_BEGIN_NAMESPACE

class BCReader
{
	// TODO: Move to separate class
	// Graph structures
	// Points contains indexes in vector of lines
	// Point represents intersection between those two lines
	struct Point
	{
		Point(cv::Point2f &p, int idx1, int idx2)
		{
			point = p;
			line1Index = idx1;
			line2Index = idx2;
			visited = false;
		}

		cv::Point2f point;
		int line1Index;
		int line2Index;
		bool visited;
	};

	// Line contains list of pointers (list::iterator) to points
	// that represent intersections
	struct Line
	{
		Line(cv::Vec4i &l)
		{
			line = l;
		}

		cv::Vec4i line;
		std::list<std::list<Point>::iterator> points;
	};

	public:
		BCReader(cv::Mat image, bool copy = false);
		BCReader(std::string filePath);
		~BCReader();

		BusinessCard getBusinessCard();
		void process();

	private:
		void clear();

		cv::Mat detectEdges(cv::Mat source);
		void findLines(cv::Mat source);
		void calculatePossibleIntersectionPoints();

		void findQuads();
		void findQuads_(int lineIndex, std::list<Point>::iterator first, Quad &workerQuad, int depth);
		void findMSERRect();

		void warpPerspective();
		void findTextBoxes();
		void classifyText(vector<shared_ptr<OCRText>> &text);

		vector<shared_ptr<OCRText>> preformOCR();

	private:
		BusinessCard _businessCard;

	private:
		cv::Mat _org;
		cv::Mat _image;
		cv::Mat _bcImg;
		cv::Mat _smallImg;

		// Lines and their relationship with points
		std::vector<Quad> _quads;
		std::vector<cv::Rect> _textBoxes;

		// Graph data holders, _lines = edges
		// _points = nodes
		std::vector<Line> _lines;
		std::list<Point> _points;

};

BCR_END_NAMESPACE

#endif // BCR_BCR_READER_H