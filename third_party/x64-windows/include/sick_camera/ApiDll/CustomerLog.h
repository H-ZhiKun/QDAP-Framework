/**	@file	CustomerLog.h
*
*	@brief	Class of output log.
*
*	@attention
*	Log and printf control
*
*
*	@copyright	Copyright 2016-2020 SICK AG. All rights reserved.
*	@author		Vision Lab, SICK GCN
*
*/


#pragma once

#include "Typedef.h"
#include <iostream>
#include <chrono>
#include <iomanip>

namespace SickCam
{

class CustomerLog
{
public:
	CustomerLog(const std::string path, const bool enShow = true, const bool enWrite = true, const int keepNMessage = 10);
	~CustomerLog();

	EXPORT_TO_DLL
	static std::string time();


	EXPORT_TO_DLL
	std::string getLastLog();

	// Overloading << 
	template<typename T>
	friend
		CustomerLog &
		operator << (CustomerLog & ths, const T & ss);


	/// this is the type of std::cout
	typedef std::basic_ostream<char, std::char_traits<char> > CoutType;

	/// this is the function signature of std::endl
	typedef CoutType& (*StandardEndLine)(CoutType&);

	/// define an operator<< to take in std::endl
	CustomerLog &
		operator << (StandardEndLine manip);

protected:
	CustomerLog() = delete;
	void _open();
	void _close();

private:

	std::ofstream	mFile;
	std::string		mPath;
	bool mEnShow;
	bool mEnWrite;

	std::vector<std::string> mLast; // size = keepNMessage
	int mLastId;
	int mKeepNMessage;
};


/////////////////////////////////////////////////////////////


template<typename T>
inline CustomerLog &
operator << (CustomerLog & ths, const T & ss)
{
	if (ths.mEnShow)
	{
		std::cout << ss;
	}

	if (ths.mEnWrite)
	{
		ths.mFile << ss;
	}

	return ths;
}

template < >
inline CustomerLog &
operator << (CustomerLog & ths, const std::stringstream & ss)
{
	if (ths.mEnShow)
	{
		std::cout << ss.str().c_str();
	}

	if (ths.mEnWrite)
	{
		ths.mFile << ss.str().c_str();
	}

	return ths;
}


}