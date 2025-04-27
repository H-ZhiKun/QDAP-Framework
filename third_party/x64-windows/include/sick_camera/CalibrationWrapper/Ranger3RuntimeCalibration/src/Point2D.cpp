#include "../include/Point2D.h"
#include <cmath>

namespace ranger3calibration {

	double Point2D::norm() const
	{
		return std::sqrt(x*x + y*y);
	}

	Point2D Point2D::normalized() const
	{
		return Point2D(x, y) / norm();
	}

	Point2D& Point2D::normalize()
	{
		double n = norm();
		x /= n;
		y /= n;
		return *this;
	}

	Point2D Point2D::operator+(const Point2D& other) const
	{
		return Point2D(x + other.x, y + other.y);
	}

	Point2D Point2D::operator-(const Point2D& other) const
	{
		return Point2D(x - other.x, y - other.y);
	}

	Point2D Point2D::operator/(const double& other) const
	{
		return Point2D(x / other, y / other);
	}

	Point2D Point2D::operator*(const double& other) const
	{
		return Point2D(x * other, y * other);
	}
	Point2D Point2D::operator*(const Point2D& other) const
	{
		return Point2D(x * other.x, y * other.y);
	}
}
