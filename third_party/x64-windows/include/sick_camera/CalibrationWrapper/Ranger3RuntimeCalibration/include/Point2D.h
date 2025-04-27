#pragma once
namespace ranger3calibration {

	class Point2D
	{
	public:
		Point2D() : x(0.0), y(0.0) {}
		Point2D(const double& x, const double& y) : x(x), y(y) {}
		double x, y;

		Point2D operator+(const Point2D& other) const;
		Point2D operator-(const Point2D& other) const;
		Point2D operator/(const double& other) const;
		Point2D operator*(const double& other) const;
		Point2D operator*(const Point2D& other) const;

		double norm() const;
		Point2D normalized() const;
		Point2D& normalize();
	private:
	};
}
