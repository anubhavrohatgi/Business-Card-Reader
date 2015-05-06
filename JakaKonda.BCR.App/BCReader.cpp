#include "BCReader.h"

using namespace std;


BCR_BEGIN_NAMESPACE

BCReader::BCReader(cv::Mat image, bool copy)
{
	if (copy)
	{
		_org = cv::Mat(image.size(), image.type());
		_org = image.clone();
	}
	else
		_org = image;
}


BCReader::~BCReader()
{
}


BCReader::BCReader(string filePath)
{
	_org = cv::imread(filePath);
}



BusinessCard BCReader::getBusinessCard()
{
	return _businessCard;
}


void BCReader::process()
{
	this->clear();
	_org = maximize_contrast(_org);
	_image = resize(_org.clone(), 800, 800);

	findLines(detectEdges(_image));
	calculatePossibleIntersectionPoints();
	findQuads();
	
	if (_quads.empty())
		findMSERRect();

	warpPerspective();

	findTextBoxes();
	auto strings = preformOCR();
	classifyText(strings);


	this->clear();

#if BCR_DEBUG != 0
	cv::waitKey(0);
#endif
}


void BCReader::clear()
{
	_points.clear();
	_lines.clear();
	_textBoxes.clear();
	_quads.clear();
}


// Detects possible edges of a business card
cv::Mat BCReader::detectEdges(cv::Mat source)
{
	const int dilation_kernel_size = 3;
	const int dilation_kernel_center = dilation_kernel_size / 2;

	cv::Mat grad;
	cv::Mat morphKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));

	cv::Mat edges = to_gray(source);
	cv::blur(edges, edges, cv::Size(3, 3));

	edges = maximize_contrast(edges);
	cv::morphologyEx(edges, grad, cv::MORPH_GRADIENT, morphKernel);
	grad = maximize_contrast(grad);
	
	cv::Mat edgesCpy;
	cv::add(edges, grad, edgesCpy, cv::noArray(), CV_16U);
	edgesCpy = maximize_contrast(edgesCpy);

	//// Reduce noise with a 3x3 kernel 
	//cv::boxFilter(edgesCpy, edgesCpy, -1, cv::Size(3, 3));

	// Canny edge detector - mark edges as white, everything else black
	edges = auto_canny(edgesCpy);

	// Dilate edges using custom kernel, for easier search
	const cv::Mat dilationKernel = cv::getStructuringElement(
		cv::MORPH_RECT,
		cv::Size(dilation_kernel_size, dilation_kernel_size),
		cv::Point(dilation_kernel_center, dilation_kernel_center));

	cv::dilate(edges, edges, dilationKernel);

#if BCR_DEBUG & BCR_DEBUG_EDGES
	cv::imshow("Detected edges", edges);
#endif

	return edges;
}


// Finds lines using HoughLinesP
// afterwards merge similar lines that are near each other
void BCReader::findLines(cv::Mat source)
{
	/**
	* Algorithm parameters explained here:
	* http://docs.opencv.org/modules/imgproc/doc/feature_detection.html?highlight=houghlinesp#houghlinesp
	*/
	const int degrees = 1;
	const int resolution_x = 1;
	const int min_intersections = 100;
	const int min_line_length = 150;
	const int max_line_gap = 15;

	vector<cv::Vec4i> lines;

	HoughLinesP(
		source,
		lines,
		resolution_x,
		deg2rad(degrees),
		min_intersections,
		min_line_length,
		max_line_gap);

	for (size_t i = 0; i < lines.size(); i++)
	{
		auto li = lines[i];

		float slopeI = slope(li);
		float nI = li[1] - slopeI*li[0];
		float x0i = li[0];

		if (slopeI != BCR_FLT_MAX)
			x0i = -nI / slopeI;

		for (size_t j = i + 1; j < lines.size(); j++)
		{
			auto lj = lines[j];

			float slopeJ = slope(lj);
			float diff = slopeI - slopeJ;
			float nJ = lj[1] - slopeI*lj[0];

			// merge similar horizontal lines
			if (abs(slopeI) <= 1)
			{
				if (abs(diff) > 0.15)
					continue;

				if (abs(nI - nJ) > 10)
					continue;

				lines.erase(lines.begin() + j--);
			}
			// merge similar vertical lines
			else
			{
				float x0j = lj[0];

				if (slopeJ != BCR_FLT_MAX)
					x0j = -nJ / slopeJ;

				if (abs(x0j - x0i) > 30)
					continue;

				//li = mergeLines(li, lj);
				lines.erase(lines.begin() + j--);
			}
		}

		_lines.push_back(li);
	}


#if BCR_DEBUG & BCR_DEBUG_LINES
	cv::Mat debugImg = source.clone();
	cv::cvtColor(debugImg, debugImg, CV_GRAY2BGR);

	// Draw detected lines on the image
	for (size_t i = 0; i < _lines.size(); i++)
	{
		cv::Vec4i l = _lines[i].line;

		cv::line(debugImg, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]),
			cv::Scalar(0, 0, 255), 3, CV_AA);
	}

	cv::imshow("Detected Lines", debugImg);
#endif
}


// Calculate all edge points (lines intersections)
// Filter them out based on intersection angle and distances
// Build a graf of connections for much faster quadrilateral search
// and result reduction
void BCReader::calculatePossibleIntersectionPoints(void)
{
	/**
	* Lines should intersect under 90° +- angle_threshold
	* It also servers as a rejection criteria to identify fake
	* intersections
	*/
	const float angle_threshold = 15.5;
	const float distance_threshold = 20;

	for (size_t i = 0; i < _lines.size(); i++)
	{
		auto li = _lines[i].line;

		float lengths[2] = {
			length(li) * 3,
			0
		};

		for (size_t j = i + 1; j < _lines.size(); j++)
		{
			auto lj = _lines[j].line;

			float angle = rad2deg(intersection_angle(li, lj));

			// Reject: _lines do not intersection near 90°
			if (abs(angle - 90) > angle_threshold)
				continue;

			cv::Point2f pt = line_intersection(li, lj);

			// Reject: invalid intersection
			if (pt.x < 0 || pt.y < 0)
				continue;

			// Distance from intersection to points to line points
			float distances [] = {
				distance(pt.x, pt.y, li[0], li[1]),
				distance(pt.x, pt.y, li[2], li[3]),
				distance(pt.x, pt.y, lj[0], lj[1]),
				distance(pt.x, pt.y, lj[2], lj[3]),
			};

			// Reject: intersection is in the middle of a line
			// It should be within the rectangle limited by points and 
			// at least distance_treshdold away from edges, 
			// to keep crucial points
			if (distances[0] > distance_threshold &&
				distances[1] > distance_threshold &&
				distances[2] > distance_threshold &&
				distances[3] > distance_threshold &&
				(
				min(li[0], li[2]) <= pt.x &&
				max(li[0], li[2]) >= pt.x &&
				min(li[1], li[3]) <= pt.y &&
				max(li[1], li[3]) >= pt.y
				||
				min(lj[0], lj[2]) <= pt.x &&
				max(lj[0], lj[2]) >= pt.x &&
				min(lj[1], lj[3]) <= pt.y &&
				max(lj[1], lj[3]) >= pt.y
				))
				continue;

			lengths[1] = length(lj) * 3;

			auto disti = min(distances[0], distances[1]);
			auto distj = min(distances[2], distances[3]);

			if (disti + distj > lengths[0] / 3 * 3 / 4 + lengths[1] / 3 * 3 / 4)
				continue;

			// It shouldn't be farther than from limiting points than 3 times 
			// of lines distance
			if (lengths[0] < distances[0] && lengths[0] < distances[1]
				||
				lengths[1] < distances[2] && lengths[1] < distances[3])
				continue;

			// Push data to graph
			_points.push_back(Point(pt, i, j));
			_lines[i].points.push_back(--_points.end());
			_lines[j].points.push_back(--_points.end());
		}
	}

#if BCR_DEBUG & BCR_DEBUG_POINTS
	cout << "Nr. of detected corners: " << _points.size() << endl;

	cv::Mat cornersImg = _image.clone();

	/// Draw detected points
	for (auto node : _points)
		cv::circle(cornersImg, node.point, 3, cv::Scalar(255, 0, 255), 2, 3);

	cv::imshow("Detected Corners", cornersImg);
#endif
}


// Recursevly traverses through graph to generate all possible quadrilaterals
// At each point one quad is marked as visited and continues with the same
// procedure on it's neighbors.
void BCReader::findQuads_(int lineIndex, list<Point>::iterator first,
	Quad &workerQuad, int depth)
{
	auto points = _lines[lineIndex].points;

	for (auto i = points.begin(); i != points.end(); ++i)
	{
		// First must must much the last one, otherwise is not 
		// a quadrilateral
		if (depth == 4)
		{
			if ((*i)->point == first->point)
			{
				auto quad = workerQuad.copy();

				if (quad.isValid() && quad.surface() > 10000)
					_quads.push_back(quad);
			}
		}
		else
		{
			if ((*i)->visited == true)
				continue;

			auto lineDst = distance(workerQuad[depth - 1], (*i)->point);

			if (lineDst < 100)
				continue;

			(*i)->visited = true;

			workerQuad.points().push_back((*i)->point);

			findQuads_((*i)->line1Index, first, workerQuad, depth + 1);
			findQuads_((*i)->line2Index, first, workerQuad, depth + 1);

			workerQuad.points().pop_back();

			(*i)->visited = false;
		}
	}
}


void BCReader::findQuads()
{
	Quad worker;

	for (auto i = _points.begin(); i != _points.end(); ++i)
	{
		i->visited = true;

		worker.points().push_back(i->point);
		findQuads_(i->line1Index, i, worker, 1);
		findQuads_(i->line2Index, i, worker, 1);
		worker.points().pop_back();
	}

	for (size_t i = 0; i < _quads.size(); i++)
		for (size_t j = i + 1; j < _quads.size(); j++)
			if (_quads[i] == _quads[j])
				_quads.erase(_quads.begin() + j--);

	// Order by surface descending
	sort(_quads.begin(), _quads.end(), [](Quad &q1, Quad &q2) {
		return q1.surface() > q2.surface();
	});

#if BCR_DEBUG & BCR_DEBUG_QUADS
	cout << "Quads found: " << _quads.size() << endl;
	auto quadsImg = _image.clone();

	// Draw all quads to the image
	for (size_t i = 0; i < _quads.size(); i++)
		for (size_t j = 0; j < 4; j++)
			cv::line(quadsImg, _quads[i][j], _quads[i][(j + 1) % 4],
			cv::Scalar(255, i * 20 + 10, 0), 2, 3);

	cv::imshow("Detected Quads", quadsImg);
#endif
}


// As quads weren't detected, use Maximally stable extremal regions
// to found bounding rect of letters
void BCReader::findMSERRect()
{
	cv::Mat mser_mask = MSER_filter(_image);

	int right = INT_MIN;
	int top = INT_MAX;
	int left = INT_MAX;
	int bottom = INT_MIN;

	int i, j;
	uchar* p;
	for (i = 0; i < mser_mask.rows; ++i)
	{
		p = mser_mask.ptr<uchar>(i);
		for (j = 0; j < mser_mask.cols; ++j)
		{
			if (p[j] == 0)
				continue;

			if (count_neighbours(mser_mask, i, j) == 0)
				continue;

			if (j < left)
				left = j;

			if (j > right)
				right = j;

			if (i > bottom)
				bottom = i;

			if (i < top)
				top = i;
		}
	}

	int scaleUpPix = 10;

	left -= scaleUpPix;
	if (left < 0)
		left = 0;

	right += scaleUpPix;
	if (right >= mser_mask.cols)
		right = mser_mask.cols - 1;

	top -= scaleUpPix;
	if (top < 0)
		top = 0;

	bottom += scaleUpPix;
	if (bottom >= mser_mask.rows)
		bottom = mser_mask.rows - 1;

	Quad q;
	q.points().push_back(cv::Point2i(left, top));
	q.points().push_back(cv::Point2i(right, top));
	q.points().push_back(cv::Point2i(right, bottom));
	q.points().push_back(cv::Point2i(left, bottom));

	_quads.push_back(q);
}



// Business Card can be rotated in 3D space.
// Warp it so it will be orthogonal to use camera
void BCReader::warpPerspective()
{
	const int pix_tresh = 10;

	auto q = _quads[0];

	auto widthRatio = static_cast<float>(_org.cols) / _image.cols;
	auto heightRatio = static_cast<float>(_org.rows) / _image.rows;

	_quads[0].scale(widthRatio, heightRatio);

	// Detect maximum height and width
	int height = max(q.leftLength(), q.rightLength())*heightRatio;
	int width = max(q.topLength(), q.bottomLength())*widthRatio;

	cv::Point2f dst [] = {
		cv::Point2f(0, 0),
		cv::Point2f(width - 1, 0),
		cv::Point2f(width - 1, height - 1),
		cv::Point2f(0, height - 1),
	};

	// Create transformation matrix
	// &vector[0] => introduce vector as an array to compiler
	auto M = cv::getPerspectiveTransform(&_quads[0].points()[0], dst);

	cv::warpPerspective(_org, _bcImg, M, cv::Size(width, height));

	// Define Region Of Interest (ROI)
	// Remove 5 pixels from sides to reduce noise from edges
	cv::Rect ROI(pix_tresh, pix_tresh,
				 width - 2 * pix_tresh, height - 2 * pix_tresh);

	_bcImg = _bcImg(ROI);
	

#if BCR_DEBUG & BCR_DEBUG_PERSP_TRANF
	auto img = resize(_bcImg, 600, 600);
	cv::imshow("Warped Business Card", img);
#endif
}


// Find text where there is rapid change in color (indicates text)
// Connect nearby detected regions, they are highly likely words.
void BCReader::findTextBoxes()
{
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;

	cv::Mat smallGray;
	cv::Mat grad;
	cv::Mat morphKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));

	// Downsample it, convert to gray and use it for processing
	_smallImg = resize(_bcImg.clone(), 400, 400);
	
	cv::cvtColor(_smallImg, smallGray, CV_BGR2GRAY);
	
	// Detect fast transitions, eg white to black (background to text)
	cv::morphologyEx(smallGray, grad, cv::MORPH_GRADIENT, morphKernel);

	// Binarize using OTSU method 
	// (automatic threshold values => appropriate for text)
	smallGray = maximize_contrast(smallGray);
	
	cv::threshold(grad, smallGray, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
	

	// Connect elements close to each another, making it appear as on element.
	morphKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9, 2));
	cv::morphologyEx(smallGray, smallGray, cv::MORPH_CLOSE, morphKernel);
	
	// Find connected elements using contours
	cv::findContours(smallGray, contours, hierarchy, CV_RETR_CCOMP, 
					 CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	
	// Filter contours
	for (int i = 0; i >= 0; i = hierarchy[i][0])
	{
		cv::Rect rect = cv::boundingRect(contours[i]);
		
		// Set region only to selected contour
		cv::Mat maskROI(smallGray, rect);
		maskROI = cv::Scalar(0, 0, 0);

		bool ok = true;

		// Textbox should contain another textbox, reject if it does
		for (auto &t : _textBoxes)
		{
			if (rect.contains(cv::Point(t.x, t.y)) &&
				rect.contains(cv::Point(t.x, t.y + t.height)) &&
				rect.contains(cv::Point(t.x + t.width, t.y)) &&
				rect.contains(cv::Point(t.x + t.width, t.y + t.height)))
			{
				ok = false;
				break;
			}
		}

		if (!ok)
			continue;

		cv::drawContours(smallGray, contours, i, cv::Scalar(255, 255, 255), CV_FILLED);
		
		// Calculate % of white pixels
		auto r = static_cast<double>(cv::countNonZero(maskROI)) / (rect.width*rect.height);

		// Reject detected text area if:
		// - Less than r% are dark pixels
		// - Detected rectangle is very small
		if (r > 0.400 && rect.height >= 3 && rect.width >= 8 && rect.area() > 60)
		{
			_textBoxes.push_back(rect);

		#if BCR_DEBUG & BCR_DEBUG_TEXT_BOXES
			rectangle(smallGray, rect, cv::Scalar(0, 255, 0), 2);
		#endif
		}
	}

#if BCR_DEBUG & BCR_DEBUG_TEXT_BOXES
	cv::imshow("Detected Text Boxes", smallGray);
#endif
}


vector<shared_ptr<OCRText>> BCReader::preformOCR()
{
	vector<shared_ptr<OCRText>> text;
	cv::Mat bcGray = to_gray(_bcImg);

	cv::Mat blured;
	//// Sharpen image
	/*cv::GaussianBlur(bcGray, blured, cv::Size(0, 0), 3);
	cv::addWeighted(bcGray, 1.50, blured, -0.50, 0, bcGray);*/
	
	auto widthRatio = bcGray.cols * 1.f / _smallImg.cols;
	auto heightRatio = bcGray.rows * 1.f / _smallImg.rows;

	OCR ocr;
	text.reserve(_textBoxes.size());

	for (auto i = _textBoxes.begin(); i != _textBoxes.end(); ++i)
	{ 
		// Scale area from small image to original
		auto rect = *i;
		i->x *= widthRatio;
		i->y *= heightRatio;
		i->width *= widthRatio;
		i->height *= heightRatio;

		// Extract it to perform OCR on it
		// Scale everything x4 for better result
		auto textImg = bcGray(*i).clone();
		textImg = resize(textImg, textImg.cols*4, textImg.rows*4);
		textImg = maximize_contrast(textImg);
		cv::Mat edges = auto_canny(textImg);
		cv::threshold(textImg, textImg, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

		float bwRatio = cv::countNonZero(textImg) * 1.f / (textImg.rows * textImg.cols);

		ocr.setImage(textImg);
 		auto txtResult = ocr.getOCRResult();

		if (txtResult->text().empty())
			continue;

		txtResult->setPos(cv::Point2i(rect.x, rect.y));
		txtResult->setSize(cv::Size(rect.width, rect.height));

		// Tesseract Lies about font size! Use Height instead 
		txtResult->fontSize = txtResult->size().height - txtResult->size().height % 2;

		text.push_back(txtResult);
	}

	return text;
}


void BCReader::classifyText(vector<shared_ptr<OCRText>> &text)
{
	BCRTextClassifier txtClass;
	txtClass.setText(text);
	txtClass.process();

	_businessCard = txtClass.bussinesCard();
	_businessCard.setImage(_bcImg);
}


BCR_END_NAMESPACE