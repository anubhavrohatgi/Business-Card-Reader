#include "Quad.h"

using namespace std;


BCR_BEGIN_NAMESPACE

Quad::Quad()
{
	// Quad always has 4 points
	_points.reserve(4);
}


Quad Quad::copy()
{
	Quad q;
	q._points = this->_points;

	return q;
}


float Quad::surface()
{
	// http://www.mathopenref.com/coordpolygonarea.html
	const auto p = _points;
	auto sum = 0.0f;

	for (auto i = 0; i < 4; i++)
		sum += (p[i].x*p[(i + 1) % 4].y - p[i].y*p[(i + 1) % 4].x);

	return abs(sum / 2);
}


void Quad::orderCorners()
{
	auto avgX = 0;
	auto avgY = 0;

	for (auto p : _points)
	{
		avgX += p.x;
		avgY += p.y;
	}

	avgX /= 4;
	avgY /= 4;

	// Order points clockwise order:
	// top left -> top right -> bottom right -> bottom left
	sort(_points.begin(), _points.end(), [=](cv::Point2f p1, cv::Point2f p2)
	{
		if (p1.y < avgY && p2.y < avgY)
			return p1.x < p2.x;

		if (p1.y > avgY && p2.y > avgY)
			return p1.x > p2.x;

		return p1.y < p2.y;
	});
}


bool Quad::operator==(Quad &q)
{
	if (this->_points.size() != q._points.size())
		return false;

	for (auto i = 0; i < _points.size(); i++)
		if (_points[i].x != q._points[i].x ||
			_points[i].y != q._points[i].y)
			return false;

	return true;
}


cv::Point2f Quad::operator[](int idx)
{
	return _points[idx];
}


vector<cv::Point2f>& Quad::points()
{
	return _points;
}


cv::Point2f Quad::topRight()
{
	return _points[0];
}


cv::Point2f Quad::topLeft()
{
	return _points[1];
}


cv::Point2f Quad::bottomLeft()
{
	return _points[2];
}


cv::Point2f Quad::bottomRight()
{
	return _points[3];
}


float Quad::topLength()
{
	return distance(topLeft(), topRight());
}


float Quad::bottomLength()
{
	return distance(bottomLeft(), bottomRight());
}


float Quad::rightLength()
{
	return distance(bottomRight(), topRight());
}


float Quad::leftLength()
{
	return distance(bottomLeft(), topLeft());
}


// Verifies that Quad would be a valid rect in 3D space
bool Quad::isValid()
{
	this->orderCorners();

	float lengths[] = {
		distance(_points[0], _points[1]),
		distance(_points[1], _points[2]),
		distance(_points[2], _points[3]),
		distance(_points[3], _points[0]),
	};

	// All lines should be at least 150px long
	for (int i = 0; i < 4; i++)
		if (lengths[i] < 100)
			return false;

	// Edges should be within 1:4 ratio
	auto ratio = leftLength() / topLength();
	if (!(0.25 < ratio && ratio <= 4))
		return false;


	float angles[] = {
		atan(slope(_points[0], _points[1])),
		atan(slope(_points[1], _points[2])),
		atan(slope(_points[2], _points[3])),
		atan(slope(_points[3], _points[0])),
	};

	// All parallel edges should +/- 20°
	if (abs(abs(angles[1]) - abs(angles[3])) > deg2rad(20) ||
		abs(abs(angles[0]) - abs(angles[2])) > deg2rad(20))
		return false;
	

	float intersectionAngles [] = {
		rad2deg(intersection_angle(_points[0], _points[1], _points[1], _points[2])),
		rad2deg(intersection_angle(_points[1], _points[2], _points[2], _points[3])),
		rad2deg(intersection_angle(_points[2], _points[3], _points[3], _points[0])),
		rad2deg(intersection_angle(_points[3], _points[0], _points[0], _points[1])),
	};

	auto countNear90 = 0;
	for (auto a : intersectionAngles)
		if (abs(90 - a) < 2)
			countNear90++;

	if (countNear90 >= 2 && countNear90 != 4)
		return false;

	float crossAngles [] = {
		intersectionAngles[0] + intersectionAngles[2],
		intersectionAngles[1] + intersectionAngles[3],
	};

	float sum = std::accumulate(crossAngles, crossAngles + 2, 0);
	
	if (!(352.5 < sum && sum < 367.5))
		return false;

	return true;
}


void Quad::scale(float widthRatio, float heightRatio)
{
	for (int i = 0; i < _points.size(); i++)
	{
		_points[i].x *= widthRatio;
		_points[i].y *= heightRatio;
	}
}

BCR_END_NAMESPACE