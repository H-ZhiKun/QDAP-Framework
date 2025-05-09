#pragma once
#include <string>
#include <vector>

//?????????????????????????????????
namespace SICK
{
	//????????��????
	enum class SensorType
	{
		UnKnownType = 0,
		LMS511 = 1,	//LMS 511
		LMS111 = 2,	//LMS 111
		LRS4000 = 3,	//LRS4000
		MULTISCAN = 4,	// Multiscan
		PICOSCAN = 5, //PicoScan150
		PICOSCAN120=6, //PicoScan120
		LMS4000=7
	};

	//???????
	struct CommonError
	{
		//?????????
		enum class ErrorCode
		{
			NO_ANYERROR = 0,  //?????
			LMS_CONNECT_FAIL = 1,  //????LMS?????????
			LMS_LOGIN_FAIL = 2,  //???LMS?????????
			LMS_CREATE_ScanTHREAD_FAIL = 3,  //????LMS?????????
			LMS_NO_SUPPORT_ERR = 9,  //????��?????��???
			LMS_NO_CONNECT_ERR=5,  //??????��????
			LMS_RECV_MESS_ERR = 6,  //???????????????
			LMS_SET_PARAM_ERR = 7,  //???????????????
			LMS_NO_ALL_ECHO_ERR = 8,  //��??????��??
			LMS_CONTAMINATION_WARN = 9, //?????????????????????????
			LMS_CONTAMINATION_ERR = 10, //?????????????????????????
		};

		ErrorCode code; //???????????
		std::string prompt; //???????????????????????????????????????????????��?????????????????????
	};

	struct Point3D
	{
		float X;
		float Y;
		float Z;  // 高度坐标
		int Distance; // 距离
		float Angle; // 角度 -180-180,水平方向称为方位角Azimuth
		float Elevation; // 仰角
		unsigned short RSSI; // 接收信号强度指示
		unsigned short LevelLable;  // 层标签
		unsigned short Reflector;  // 反射器

		Point3D()
			: X(0), Y(0), Z(0), Distance(0), Angle(0), Elevation(0), RSSI(0), LevelLable(0), Reflector(0) {}

		Point3D(float x, float y, float z, int distance, float angle, float elevation, unsigned short rssi, unsigned short levelLable, unsigned short reflector)
			: X(x), Y(y), Z(z), Distance(distance), Angle(angle), Elevation(elevation), RSSI(rssi), LevelLable(levelLable), Reflector(reflector) {}

		Point3D(const Point3D& p)
			: X(p.X), Y(p.Y), Z(p.Z), Distance(p.Distance), Angle(p.Angle), Elevation(p.Elevation), RSSI(p.RSSI), LevelLable(p.LevelLable), Reflector(p.Reflector) {}

		Point3D& operator=(const Point3D& p)
		{
			if (this != &p) {
				X = p.X; Y = p.Y; Z = p.Z; Distance = p.Distance; Angle = p.Angle; Elevation = p.Elevation; RSSI = p.RSSI; LevelLable = p.LevelLable; Reflector = p.Reflector;
			}
			return *this;
		}

		void clear()
		{
			X = 0; Y = 0; Z = 0; Distance = 0; Angle = 0; Elevation = 0; RSSI = 0; LevelLable = 0; Reflector = 0;
		}

		bool operator==(const Point3D& p) const
		{
			return (X == p.X && Y == p.Y && Z == p.Z && Distance == p.Distance && Angle == p.Angle && Elevation == p.Elevation && RSSI == p.RSSI && LevelLable == p.LevelLable && Reflector == p.Reflector);
		}

		bool operator<(const Point3D& p) const
		{
			if (X != p.X) return X < p.X;
			if (Y != p.Y) return Y < p.Y;
			if (Z != p.Z) return Z < p.Z;
			if (Distance != p.Distance) return Distance < p.Distance;
			if (Angle != p.Angle) return Angle < p.Angle;
			if (Elevation != p.Elevation) return Elevation < p.Elevation;
			if (RSSI != p.RSSI) return RSSI < p.RSSI;
			if (LevelLable != p.LevelLable) return LevelLable < p.LevelLable;
			return Reflector < p.Reflector;
		}
	};

	typedef std::vector<Point3D> Single3DPointVec;

	struct ProfileInformation
	{
		float startAngle;    //??????
		float stopAngle;    //???????
		unsigned long long frameNumer;    // ??????????
		unsigned long long timeStampStart;  //???????
		unsigned long long timeStampStop;  //??????????????????????��??????
		unsigned long long timeStampTransmit; //??????segment??transmit
		unsigned int encodePosition;   //??????��???????????????��??????
		unsigned short encoderSpeed;   //??????????????????????��??????
		Single3DPointVec points;    //???????????

		void clear()
		{
			startAngle = 0.0f;
			stopAngle = 0.0f;
			frameNumer = 0;
			timeStampStart = 0;
			timeStampStop = 0;
			encodePosition = 0;
			encoderSpeed = 0;
			points.clear();
		}
	};
};

