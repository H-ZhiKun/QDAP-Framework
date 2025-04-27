#include "CustomerLog.h"


namespace SickCam 
{

CustomerLog::CustomerLog(const std::string path, const bool enShow, const bool enWrite, const int keepNMessage)
	: mEnShow(enShow)
	, mEnWrite(enWrite)
	, mLast(10)
	, mLastId(0)
	, mKeepNMessage(keepNMessage > 1 ? keepNMessage : 1)
{
	if (mEnWrite && !path.empty())
	{
		mPath = path;
		mFile.open(mPath, std::fstream::app);
	}
	else
	{
		mEnWrite = false;
	}
}

CustomerLog::~CustomerLog()
{
	if (mEnWrite)
		mFile.close();
}

std::string
CustomerLog::time()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t(now);
	struct tm ltm = { 0 };
#ifdef __linux__
    localtime_r(&tt, &ltm); // ubuntu
#else
    localtime_s(&ltm, &tt);
#endif
    std::stringstream stm;
	stm << std::setfill('0');
	stm << std::setw(4) << (ltm.tm_year + 1900) << "-";
	stm << std::setw(2) << (ltm.tm_mon + 1) << "-";
	stm << std::setw(2) << (ltm.tm_mday) << " ";
	stm << std::setw(2) << (ltm.tm_hour) << ":";
	stm << std::setw(2) << (ltm.tm_min) << ":";
	stm << std::setw(2) << (ltm.tm_sec);

	return stm.str();
}

std::string 
CustomerLog::getLastLog()
{
	std::stringstream ss;
	for (int i = mLastId; i < mKeepNMessage; ++i)
		ss << mLast[i];

	for (int i = 0; i < mLastId; ++i)
		ss << mLast[i];

	return ss.str();
}


void
CustomerLog::_open()
{
	if (mEnWrite)
	{
		if (!mFile.is_open())
			mFile.open(mPath, std::fstream::app);
	}
}

void
CustomerLog::_close()
{
	if (mEnWrite)
	{
		if (mFile.is_open())
			mFile.close();
	}
}

CustomerLog &
CustomerLog::operator << (StandardEndLine manip)
{
	if (manip == static_cast<StandardEndLine>(endl))
	{
		if (mEnShow)
		{
			cout << "\n\n\n" << "----- "<< time() <<" -----\n" << endl;
		}

		if (mEnWrite)
		{
			_open();
			mFile << "\n\n\n" << "----- " << time() << " -----\n" << endl;
			_close();
		}

		std::stringstream ss;
		ss << "\n\n\n" << "----- " << time() << " -----\n" << endl;
		mLast[mLastId++] = ss.str();
		mLastId %= mKeepNMessage;
	}
	/// call the function, but we cannot return it's value
	manip(cout);
	return *this;
}




}

