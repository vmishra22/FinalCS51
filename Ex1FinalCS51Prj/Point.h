#ifndef POINT_H_
#define POINT_H_

#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
using namespace std;

template <typename T, size_t N>
class Point
{
public:
	Point(){
		fill_n(mPosition, N, T());
	}
	Point(const T& pX) : mPosition[0](pX){
		assert(N == 1);
	}
	Point(const T& pX, const T& pY) {
		assert(N == 2);
		mPosition[0] = pX;
		mPosition[1] = pY;
	}
	Point(vector<T>& vPos){
		assert(N == vPos.size());
		int count = 0;
		for (vector<T>::iterator it = vPos.begin(); it != vPos.end(); ++it, ++count)
			mPosition[count] = *it;
	}
	T& operator[](int i){
		assert(i < N);
		return mPosition[i];
	}
	T const& operator[](int i) const{
		assert(i < N);
		return mPosition[i];
	}
	void operator+=(const Point& iPoint){
		for (int i = 0; i < N; ++i)
			mPosition[i] += iPoint[i];
	}
	void operator-=(const Point& iPoint){
		for (int i = 0; i < N; ++i)
			mPosition[i] -= iPoint[i];
	}

private:
	T mPosition[N];
};


typedef Point< float, 2 > PointFloat2D;
typedef Point< float, 3 > PointFloat3D;



#endif //POINT_H_