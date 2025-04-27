#include "../include/Transformations.h"

namespace ranger3calibration {
	void quickScheimpflugInverse(const double& sch_sx, const double& sch_sy, double* SchInv)
	{
#if SHEIMPFLUG_MODEL == 1
		SchInv[0] = 1.0 - sch_sy*sch_sy;
		SchInv[1] = sch_sx*sch_sy;
		SchInv[2] = -sch_sx;

		SchInv[3] = SchInv[1];
		SchInv[4] = 1.0 - sch_sx*sch_sx;
		SchInv[5] = -sch_sy;

		SchInv[6] = SchInv[2];
		SchInv[7] = SchInv[5];
		SchInv[8] = 1.0;
#elif SHEIMPFLUG_MODEL == 2
		// Temporary hack
		double Htemp[9];
		std::memset(Htemp, 0, sizeof(double) * 9);
		Htemp[0] = Htemp[4] = Htemp[8] = 1.0;
		Htemp[6] = sch_sx;
		Htemp[7] = sch_sy;
		quickHomographyInverse(Htemp, SchInv);
#endif
	}

	void quickHomographyInverse(const double* H, double* Hinv)
	{
		const double& a = H[0];
		const double& b = H[1];
		const double& c = H[2];
		const double& d = H[3];
		const double& e = H[4];
		const double& f = H[5];
		const double& g = H[6];
		const double& h = H[7];
		const double& i = H[8];

		// Todo: decide if we should set last element to one or not..
		// it's probably good enough to do this...
		double scale = a * e - b * d;
		Hinv[0] = (e * i - f * h) / scale;
		Hinv[1] = (c * h - b * i) / scale;
		Hinv[2] = (b * f - c * e) / scale;
		Hinv[3] = (f * g - d * i) / scale;
		Hinv[4] = (a * i - c * g) / scale;
		Hinv[5] = (c * d - a * f) / scale;
		Hinv[6] = (d * h - e * g) / scale;
		Hinv[7] = (b * g - a * h) / scale;
		//Hinv[8] = (a * e - b * d) / scale;
		Hinv[8] = 1.0;
	}

	
	Point2D sensorToWorld(const Point2D& sp, const double* const lens, const double& sch_sx /*roll*/, const double& sch_sy /*tilt*/, const double* const H)
	{
		// Subtract lens center
		double ud = (sp.x - lens[0]) / LENS_PRESCALER;
		double vd = (sp.y - lens[1]) / LENS_PRESCALER;

		// Apply Scheimpflug projection
		applyScheimpflug(sch_sx, sch_sy, ud, vd);

		// Undistort points
		double u, v;
		applyLensCorrection(lens, ud, vd, u, v);

		// Project into world
		double d = H[6] * u + H[7] * v + H[8];
		double xp = (H[0] * u + H[1] * v + H[2]) / d;
		double yp = (H[3] * u + H[4] * v + H[5]) / d;

		return Point2D(xp, yp);
	}

	Point2D worldToSensor(const Point2D& sp, const double* const lens, const double& sch_sx, const double& sch_sy, const double* const H)
	{
		double Hinv[9];
		double Sinv[9];
		quickHomographyInverse(H, Hinv);
		quickScheimpflugInverse(sch_sx, sch_sy, Sinv);
		return worldToSensor(sp, lens, Sinv, Hinv);
	}

	Point2D worldToSensor(const Point2D& wp, const double* const lens, const double* SchInv, const double* const Hinv)
	{
		// Project from world
		double d = Hinv[6] * wp.x + Hinv[7] * wp.y + Hinv[8];
		double ud = (Hinv[0] * wp.x + Hinv[1] * wp.y + Hinv[2]) / d;
		double vd = (Hinv[3] * wp.x + Hinv[4] * wp.y + Hinv[5]) / d;

		// Distort points
		double u, v;
		applyLensDistortion(lens, ud, vd, u, v);

		// Apply scheimplflug inverse
		d = u*SchInv[6] + v*SchInv[7] + SchInv[8];
		double spu = (u*SchInv[0] + v*SchInv[1] + SchInv[2]) / d;
		double spv = (u*SchInv[3] + v*SchInv[4] + SchInv[5]) / d;

		// Add lens center
		spu = spu*LENS_PRESCALER + lens[0];
		spv = spv*LENS_PRESCALER + lens[1];

		return Point2D(spu, spv);
	}

	void sensorToWorld(const std::vector<Point2D>& sp, std::vector<Point2D>& wp, const double* const lensSensorToWorld, const double& sch_sx, const double& sch_sy, const double* const H)
	{
		if (&wp != &sp)
			wp = sp;
		for (std::vector<Point2D>::iterator it = wp.begin(); it != wp.end(); ++it)
			*it = sensorToWorld(*it, lensSensorToWorld, sch_sx, sch_sy, H);

	}
	void worldToSensor(const std::vector<Point2D>& wp, std::vector<Point2D>& sp, const double* const lensWorldToSensor, const double& sch_sx, const double& sch_sy, const double* const H)
	{
		double Hinv[9];
		double Sinv[9];
		quickHomographyInverse(H, Hinv);
		quickScheimpflugInverse(sch_sx, sch_sy, Sinv);

		if (&wp != &sp)
			sp = wp;
		for (std::vector<Point2D>::iterator it = sp.begin(); it != sp.end(); ++it)
			*it = worldToSensor(*it, lensWorldToSensor, Sinv, Hinv);
	}
}
