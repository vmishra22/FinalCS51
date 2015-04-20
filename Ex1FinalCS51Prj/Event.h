#ifndef EVENT_H_
#define EVENT_H_

#include <iostream>
#include "Point.h"
#include "Line.h"
using namespace std;

template <typename T, size_t N>
class Event
{
public:
	Event()
	{
		mIsLeftPoint = false;
		mEventPoint = Point<T, N>::Point();
		mSegment = Line<T, N>::Point();
	}

	Event(const Point<T, N>& iPoint, const Line<T, N>& iLine, bool iPointSide = false)
	{
		mIsLeftPoint = iPointSide;
		mEventPoint = iPoint;
		mSegment = iLine;
	}

	bool getLeftStatus()
	{
		return mIsLeftPoint;
	}

	const Point<T, N>& getEventPoint()
	{
		return mEventPoint;
	}

	const Line<T, N>& getSegment()
	{
		return mSegment;
	}

private:
	bool mIsLeftPoint;
	Point<T, N> mEventPoint;
	Line<T, N> mSegment;
};

typedef Event< float, 2 > EventFloat2D;
typedef Event< float, 3 > EventFloat3D;


#endif //EVENT_H_