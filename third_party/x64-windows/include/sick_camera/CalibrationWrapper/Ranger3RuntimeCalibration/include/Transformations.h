#pragma once 
#include "Point2D.h"
#include "Settings.h"
#ifndef __linux__
#include <immintrin.h>
#endif
#include <vector>

#ifdef __linux__
#   define EXPORT_TO_DLL

#else
#if defined(DLL_EXPORTS) // inside DLL
#   define EXPORT_TO_DLL   __declspec(dllexport)
#else // outside DLL
#   define EXPORT_TO_DLL   __declspec(dllimport)
#endif  

#endif // __linux__


namespace ranger3calibration {
#ifndef __linux__
	class Vector4d {
	public:
		inline Vector4d() {}
		inline Vector4d(const double val) : reg(_mm256_set1_pd(val)){}
		inline Vector4d(const double val1, const double val2, const double val3, const double val4) : reg(_mm256_set_pd(val4, val3, val2, val1)){}
		inline Vector4d(const __m256d& rhs) : reg(rhs){}

		inline Vector4d& operator=(const __m256d& rhs) {
			reg = rhs;
			return *this;
		}

		inline operator __m256d() const { return reg; } //Conversion function

		inline Vector4d& operator+=(const Vector4d& rhs) {
			*this = _mm256_add_pd(*this, rhs);
			return *this;
		}

		inline Vector4d select(const Vector4d& lhs, const Vector4d mask) {
			return _mm256_blendv_pd(*this,lhs,mask);
		}

	private:
		__m256d reg;
	};
	
	// + operator
	inline Vector4d operator+(const Vector4d& lhs, const Vector4d& rhs) {
		return _mm256_add_pd(lhs, rhs);
	}
	// - operator
	inline Vector4d operator-(const Vector4d& lhs, const Vector4d& rhs) {
		return _mm256_sub_pd(lhs, rhs);
	}
	// / operator
	inline Vector4d operator/(const Vector4d& lhs, const Vector4d& rhs) {
		return _mm256_div_pd(lhs, rhs);
	}
	// * operator
	inline Vector4d operator*(const Vector4d& lhs, const Vector4d& rhs) {
		return _mm256_mul_pd(lhs, rhs);
	}

	// ==
	inline Vector4d operator==(const Vector4d& lhs, const Vector4d& rhs) {
		return _mm256_cmp_pd(lhs, rhs, 0); //_CMP_EQ_OQ
	}
	
	// != 
	inline Vector4d operator!=(const Vector4d& lhs, const Vector4d& rhs) {
		return _mm256_cmp_pd(lhs, rhs, 12); //_CMP_NEQ_OQ
	}

	// < 
	inline Vector4d operator<(const Vector4d& lhs, const Vector4d& rhs) {
		return _mm256_cmp_pd(lhs, rhs, 17); // _CMP_LT_OQ
	}

	// <= 
	inline Vector4d operator<=(const Vector4d& lhs, const Vector4d& rhs) {
		return _mm256_cmp_pd(lhs, rhs, 18); // _CMP_LE_OQ
	}

	// > 
	inline Vector4d operator>(const Vector4d& lhs, const Vector4d& rhs) {
		return rhs <= lhs;
	}

	// >=
	inline Vector4d operator>=(const Vector4d& lhs, const Vector4d& rhs) {
		return rhs < lhs;
	}

#endif

	template<typename T>
	inline void applyScheimpflug(const T& sch_sx, const T& sch_sy, T& ud, T& vd)
	{
#if SHEIMPFLUG_MODEL == 1
		// Apply Scheimpflug projection
		T d = ud * sch_sx + vd * sch_sy + T(1);
		ud = (ud + sch_sx) / d;
		vd = (vd + sch_sy) / d;
#elif SHEIMPFLUG_MODEL == 2
		// Apply Scheimpflug projection
		T d = ud * sch_sx + vd * sch_sy + T(1);
		ud = ud / d;
		vd = vd / d;
#endif
	}

	template<typename T>
	inline void applyLensCorrection(const T* const lens, const T& ud, const T& vd, T& u, T& v)
	{
#if LENS_N_RADIAL == 2
		double ud2 = ud * ud;
		double vd2 = vd * vd;
		double udvd = ud * vd;

		T r2 = ud2 + vd2;
		T radial = T(1) + r2 * (lens[2] + r2 * lens[3]);
		u = ud * radial + (T(2) * lens[4] * udvd + lens[5] * (r2 + T(2) * ud2));
		v = vd * radial + (T(2) * lens[5] * udvd + lens[4] * (r2 + T(2) * vd2));
#elif LENS_N_RADIAL == 3
		T r2 = ud * ud + vd * vd;
		T radial = T(1.0) + r2 * (lens[2] + r2 * (lens[3] + r2 * lens[4]));
		u = ud * radial + (T(2) * lens[5] * ud*vd + lens[6] * (r2 + T(2) * ud*ud));
		v = vd * radial + (T(2) * lens[6] * ud*vd + lens[5] * (r2 + T(2) * vd*vd));
#else

#endif
	}

	template <typename T>
	void applyLensDistortion(const T* const lens, const T& ud, const T& vd, T& u, T& v)
	{
		// Distort points
		u = ud;
		v = vd;
		for (int i = 0; i < 20; ++i) {
			T r2 = u * u + v * v;
#if LENS_N_RADIAL == 2
			T radial = T(1) + r2 * (lens[2] + r2 * lens[3]);
			u = (ud - (T(2) * lens[4] * u*v + lens[5] * (r2 + T(2) * u*u))) / radial;
			v = (vd - (T(2) * lens[5] * u*v + lens[4] * (r2 + T(2) * v*v))) / radial;
#elif LENS_N_RADIAL == 3
			T radial = T(1) + r2 * (lens[2] + r2 * (lens[3] + r2 * lens[4]));
			u = (ud - (T(2) * lens[5] * u*v + lens[6] * (r2 + T(2) * u*u))) / radial;
			v = (vd - (T(2) * lens[6] * u*v + lens[5] * (r2 + T(2) * v*v))) / radial;
#else

#endif
		}
	}

#ifndef __linux__
//AVX versions of v2 application only.
	inline void applyLensCorrection(const Vector4d* lens, const Vector4d ud, const Vector4d vd, Vector4d& u, Vector4d& v) {
		Vector4d ud2 = ud * ud;
		Vector4d vd2 = vd * vd;
		Vector4d udvd = ud * vd;

		Vector4d r2 = ud2 + vd2;
		Vector4d radial = Vector4d{1}+ r2 * (lens[2] + r2 * lens[3]);
		u = ud * radial + (Vector4d{2} * lens[4] * udvd + lens[5] * (r2 + Vector4d{2} * ud2));
		v = vd * radial + (Vector4d{2} * lens[5] * udvd + lens[4] * (r2 + Vector4d{2} * vd2));
	}

	inline void applyScheimpflug(const Vector4d sch_sx, const Vector4d sch_sy, Vector4d& ud, Vector4d& vd) {
		// Apply Scheimpflug projection
		Vector4d d = ud * sch_sx + vd * sch_sy + Vector4d{ 1 };
		ud = (ud + sch_sx) / d;
		vd = (vd + sch_sy) / d;
	}

	inline void sensorToWorld(Vector4d& x, Vector4d& y, const Vector4d* lens, const Vector4d sch_sx, const Vector4d sch_sy, const Vector4d* H) {
		// Subtract lens center
		Vector4d lens_prescaler{ LENS_PRESCALER };
		Vector4d ud = (x - lens[0]) / lens_prescaler;
		Vector4d vd = (y - lens[1]) / lens_prescaler;

		// Apply Scheimpflug projection
		applyScheimpflug(sch_sx, sch_sy, ud, vd);

		// Undistort points
		Vector4d u, v;
		applyLensCorrection(lens, ud, vd, u, v);

		// Project into world
		Vector4d d = H[6] * u + H[7] * v + H[8];
		x = (H[0] * u + H[1] * v + H[2]) / d;
		y = (H[3] * u + H[4] * v + H[5]) / d;
	}
#endif

	void quickScheimpflugInverse(const double& sch_roll, const double& sch_tilt, double* SchInv);

	void quickHomographyInverse(const double* H, double* Hinv);

	EXPORT_TO_DLL Point2D sensorToWorld(const Point2D& sp, const double* const lens, const double& sch_roll, const double& sch_tilt, const double* const H);

	// Convenient, but slower version
	Point2D worldToSensor(const Point2D& sp, const double* const lens, const double& sch_roll, const double& sch_tilt, const double* const H);

	// Optimized version
	Point2D worldToSensor(const Point2D& wp, const double* const lens, const double* SchInv, const double* const Hinv);

	EXPORT_TO_DLL void sensorToWorld(const std::vector<Point2D>& sp, std::vector<Point2D>& wp, const double* const lensSensorToWorld, const double& sch_roll, const double& sch_tilt, const double* const H);

	void worldToSensor(const std::vector<Point2D>& wp, std::vector<Point2D>& sp, const double* const lensWorldToSensor, const double& sch_roll, const double& sch_tilt, const double* const H);
}
