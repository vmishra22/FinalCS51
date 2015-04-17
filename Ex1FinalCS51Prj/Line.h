#ifndef LINE_H_
#define LINE_H_

#include <iostream>
#include <vector>
#include "Point.h"
using namespace std;

template <typename T, size_t N>
class Line
{
public:
	Line()
	{
		mStart = Point<T, N>::Point();
		mEnd = Point<T, N>::Point();
	}

	Line(const Point<T, N>& point1, const Point<T, N>& point2)
	{
		mStart = point1;
		mEnd = point2;
	}

	void operator+=(const Line& iLine)
	{
		mStart += iLine.getStart();
		mEnd += iLine.getEnd();
	}

	void operator-=(const Line& iLine)
	{
		mStart -= iLine.getStart();
		mEnd -= iLine.getEnd();
	}

private:
	Point<T, N> mStart;
	Point<T, N> mEnd;
};

typedef Line< float, 2 > LineFloat2D;
typedef Line< float, 3 > LineFloat3D;


#endif //LINE_H_