#include "Point.h"
#include "Line.h"
#include "Event.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <iterator>
#include <stack>
#include <queue>
#include <set>
using namespace std;

#define EPS 1.0e-10

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

GLdouble width, height;   /* window width and height */
int wd;                   /* GLUT window handle */
PointFloat2D eventPointX;

/******Utility functions***********/
bool checkFloatEquality(float a, float b)
{
	return ((fabs(a - b) < EPS) && (fabs(a - b) >= 0));
}

/**************************************************/
/***************CONVEX HULL*************************/
/*************************************************/

//Minimum Y element, if there is a tie then lesser X value element is before the other one.
bool compare_MinY(const PointFloat2D& a, const PointFloat2D& b)
{
	bool retVal = false;
	if ( (fabs(a[1] - b[1]) < EPS) && (fabs(a[1] - b[1]) >= 0) && ((a[0] - b[0]) < EPS) )
		retVal = true;
	else if ((a[1] - b[1] < 0) && (fabs(a[1] - b[1]) > EPS))
		retVal = true;
	else 
		retVal = false;
	return retVal;
}

//compare the direction of two line segments based on cross product of two vectors, 
//origin of both vectors is at (0,0)
bool compareDirection(const PointFloat2D& vect1, const PointFloat2D& vect2)
{
	float value = (vect1[0] * vect2[1] - vect2[0] * vect1[1]);
	return !(value <= 0);
}

//Check if point p1 makes a non-left turn.
bool isNonLeftTurn(const PointFloat2D& pt0, const PointFloat2D& pt1, const PointFloat2D& pt2)
{
	PointFloat2D vect1, vect2;
	vect1[0] = pt1[0] - pt0[0]; 
	vect1[1] = pt1[1] - pt0[1];
	vect2[0] = pt2[0] - pt0[0];
	vect2[1] = pt2[1] - pt0[1];

	float value = (vect2[0] * vect1[1] - vect1[0] * vect2[1]);

	return (value > 0);
}

void nextToStackTop(stack<PointFloat2D>& iStack, PointFloat2D& oPoint)
{
	if (!iStack.empty())
	{
		PointFloat2D currentTop = iStack.top();
		iStack.pop();
		oPoint = iStack.top();
		iStack.push(currentTop);
	}
}

//draw input points for finding the Convex Hull.
void drawPoints(vector<PointFloat2D>& iListPoints)
{
	glPointSize(5.);
	glBegin(GL_POINTS);
	glColor3f(1.0, 0.0, 0.0);
	glEnable(GL_POINT_SMOOTH);

	for (vector<PointFloat2D>::iterator it = iListPoints.begin(); it != iListPoints.end(); ++it)
	{
		PointFloat2D point = *it;
		glVertex2f(point[0], point[1]);
	}

	glDisable(GL_POINT_SMOOTH);
	glEnd();
	glFlush();
}

//Find convex hull points.
void findAndDrawConvexHull(vector<PointFloat2D>& iListPoints)
{
	//Put min-y element at start of the container
	sort(begin(iListPoints), end(iListPoints), compare_MinY);

	//Sort the points based on counterclock rotation of polar angles.
	vector<PointFloat2D>::iterator vecPointsItr = begin(iListPoints);
	PointFloat2D referencePoint = *vecPointsItr;
	for_each(next(vecPointsItr), iListPoints.end(), [referencePoint](PointFloat2D& pt){ pt -= referencePoint; });
	sort(next(vecPointsItr), end(iListPoints), compareDirection);
	for_each(next(vecPointsItr), iListPoints.end(), [referencePoint](PointFloat2D& pt){ pt += referencePoint; });

	//Create a stack for convex hull points.
	stack<PointFloat2D> hullStack;

	//Push first three points into stack.
	hullStack.push(referencePoint);
	hullStack.push(*(next(vecPointsItr)));
	hullStack.push(*(next(next(vecPointsItr))));

	for (vector<PointFloat2D>::iterator it = next(vecPointsItr, 3); it != iListPoints.end(); ++it)
	{
		PointFloat2D pt1, pt2, pt3;
		pt3 = *it;
		bool nonLeftTurn = true;
		while (nonLeftTurn)
		{
			nextToStackTop(hullStack, pt1);
			pt2 = hullStack.top();
			nonLeftTurn = isNonLeftTurn(pt1, pt2, pt3);
			if (nonLeftTurn)
				hullStack.pop();
		}
		hullStack.push(pt3);
	}

	glLineWidth(1.5);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glBegin(GL_LINE_LOOP);
	glColor3f(0.0, 1.0, 0.0);
	while (!hullStack.empty())
	{
		PointFloat2D pt = hullStack.top();
		glVertex2f(pt[0], pt[1]);
		hullStack.pop();
	}
	glEnd();
	glFlush();
}
/**************************************************/
/***************LINE INTERSECTION*********************/
/*************************************************/
void drawLines(vector<PointFloat2D>& iListPoints)
{
	glPointSize(5.);
	glBegin(GL_LINES);
	glColor3f(0.0, 0.0, 1.0);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glLineWidth(1.5);

	for (vector<PointFloat2D>::iterator it = iListPoints.begin(); it != iListPoints.end() && (next(it) != iListPoints.end()); it = next(it, 2))
	{
		PointFloat2D point1 = *it;
		PointFloat2D point2 = *(next(it));
		glVertex2f(point1[0], point1[1]);
		glVertex2f(point2[0], point2[1]);
	}
	glEnd();
	glFlush();
}

//create Input Line segments
void createEvents(vector<PointFloat2D>& iListInputPointsFloat2D, vector<EventFloat2D>& oEventVec)
{
	for (vector<PointFloat2D>::iterator it = iListInputPointsFloat2D.begin(); it != iListInputPointsFloat2D.end() &&
													  (next(it) != iListInputPointsFloat2D.end()); it = next(it, 2))
	{
		PointFloat2D leftPoint = *it;
		PointFloat2D rightPoint = *(next(it));
		if ((leftPoint[0] - rightPoint[0]) > 0)
		{
			PointFloat2D temp = rightPoint;
			rightPoint = leftPoint;
			leftPoint = temp;
		}
		LineFloat2D lineSegment = LineFloat2D(leftPoint, rightPoint);

		oEventVec.push_back(EventFloat2D(leftPoint, lineSegment, true));
		oEventVec.push_back(EventFloat2D(rightPoint, lineSegment, false));
	}
}

float directionOfPoints(const PointFloat2D& pi, const PointFloat2D& pj, const PointFloat2D& pk)
{
	PointFloat2D vect1, vect2;
	vect1[0] = pj[0] - pi[0];
	vect1[1] = pj[1] - pi[1];
	vect2[0] = pk[0] - pi[0];
	vect2[1] = pk[1] - pi[1];

	float value = (vect2[0] * vect1[1] - vect1[0] * vect2[1]);
	return value;
}

bool checkOnSegment(const PointFloat2D& pi, const PointFloat2D& pj, const PointFloat2D& pk)
{
	if ((min(pi[0], pj[0]) <= pk[0]) && (pk[0] <= max(pi[0], pj[0]))
		&& (min(pi[1], pj[1]) <= pk[1]) && (pk[1] <= max(pi[1], pj[1])))
		return true;
	else
		return false;
}

bool checkSegmentIntersection(LineFloat2D l1, LineFloat2D l2)
{
	PointFloat2D p1 = l1.getLeft();
	PointFloat2D p2 = l1.getRight();
	PointFloat2D p3 = l2.getLeft();
	PointFloat2D p4 = l2.getRight();

	float d1 = directionOfPoints(p3, p4, p1);
	float d2 = directionOfPoints(p3, p4, p2);
	float d3 = directionOfPoints(p1, p2, p3);
	float d4 = directionOfPoints(p1, p2, p4);

	if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
		((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0)))
		return true;
	else if (checkFloatEquality(d1, 0.) && checkOnSegment(p3, p4, p1))
		return true;
	else if (checkFloatEquality(d2, 0.) && checkOnSegment(p3, p4, p2))
		return true;
	else if (checkFloatEquality(d3, 0.) && checkOnSegment(p1, p2, p3))
		return true;
	else if (checkFloatEquality(d4, 0.) && checkOnSegment(p1, p2, p4))
		return true;
	else
		return false;

	return false;
}

class SegmentComparator
{
public:
	bool operator()(LineFloat2D l1, LineFloat2D l2)
	{
		float eventPointXVal = eventPointX[0];
		LineFloat2D sweepLine(PointFloat2D(eventPointXVal, 0.0), PointFloat2D(eventPointXVal, 10000.0));
		PointFloat2D l1Left = l1.getLeft();
		PointFloat2D l1Right = l1.getRight();
		PointFloat2D l2Left = l2.getLeft();
		PointFloat2D l2Right = l2.getRight();

		if (checkSegmentIntersection(l1, sweepLine) && checkSegmentIntersection(l2, sweepLine))
		{
			float l1YVal = (((l1Right[1] - l1Left[1]) / (l1Right[0] - l1Left[0]))* (eventPointXVal - l1Left[0])) + l1Left[1];
			float l2YVal = (((l2Right[1] - l2Left[1]) / (l2Right[0] - l2Left[0]))* (eventPointXVal - l2Left[0])) + l2Left[1];

			if ((l1YVal - l2YVal) > EPS)
				return true;
		}
		
		return false;
	}
};

class EventsComparator
{
public:
	bool operator()(EventFloat2D& e1, EventFloat2D& e2)
	{
		bool retVal = false;
		float e1X = (e1.getEventPoint())[0]; float e1Y = (e1.getEventPoint())[1];
		float e2X = (e2.getEventPoint())[0]; float e2Y = (e2.getEventPoint())[1];
		if ((e1X - e2X > 0) && (fabs(e1X - e2X) > EPS))
			retVal = true;
		else if ((fabs(e1X - e2X) < EPS) && (fabs(e1X - e2X) >= 0))
		{
			if (e1Y - e2Y > 0)
				retVal = true;
		}
		else
			retVal = false;
		return retVal;
	}
};

void drawIntersectingLines(set<LineFloat2D, bool(*)(LineFloat2D, LineFloat2D)>& iLinesSet)
{
	glPointSize(5.);
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glLineWidth(1.5);

	for (set<LineFloat2D>::iterator it = iLinesSet.begin(); it != iLinesSet.end(); ++it)
	{
		LineFloat2D lineSegment = *it;
		PointFloat2D point1 = lineSegment.getLeft();
		PointFloat2D point2 = lineSegment.getRight();
		glVertex2f(point1[0], point1[1]);
		glVertex2f(point2[0], point2[1]);
	}
	glEnd();
	glFlush();
}

bool fncomp(LineFloat2D lhs, LineFloat2D rhs) { return (lhs.getLeft())[0]<(rhs.getLeft())[0]; }

void findIntersection(vector<PointFloat2D>& iListPoints)
{
	vector<EventFloat2D> eventVector;
	createEvents(iListPoints, eventVector);

	//Create a Priority queue of min-heap for X-coordinate of event
	typedef priority_queue<EventFloat2D, vector<EventFloat2D>, EventsComparator> eventsPQ;
	eventsPQ sweepLineEventQueue(EventsComparator(), eventVector);
	
	typedef set<LineFloat2D, SegmentComparator> segmentLabelSet;
	typedef set<LineFloat2D, SegmentComparator>::iterator segmentLabelSetIter;

	segmentLabelSet labelSet;

	bool(*fn_pt)(LineFloat2D, LineFloat2D) = fncomp;
	set<LineFloat2D, bool(*)(LineFloat2D, LineFloat2D)> intersectingLines(fn_pt);

	while (!sweepLineEventQueue.empty())
	{
		EventFloat2D minEvent = sweepLineEventQueue.top();
		const LineFloat2D& lineCurrent = minEvent.getSegment();
		eventPointX = minEvent.getEventPoint();
		if (minEvent.getLeftStatus())
		{
			labelSet.insert(lineCurrent);
			segmentLabelSetIter itCurrent = labelSet.find(lineCurrent);
			if (itCurrent != labelSet.end())
			{
				segmentLabelSetIter itAbove = prev(itCurrent);
				segmentLabelSetIter itBelow = next(itCurrent);
				if (itAbove != labelSet.end())
				{
					const LineFloat2D& lineAbove = *itAbove;
					if (checkSegmentIntersection(lineCurrent, lineAbove))
					{
						intersectingLines.insert(lineCurrent);
						intersectingLines.insert(lineAbove);
					}
				}
				if (itBelow != labelSet.end())
				{
					const LineFloat2D& lineBelow = *itBelow;
					if (checkSegmentIntersection(lineCurrent, lineBelow))
					{
						intersectingLines.insert(lineCurrent);
						intersectingLines.insert(lineBelow);
					}
				}
			}
		}
		else
		{
			segmentLabelSetIter itCurrent = labelSet.find(lineCurrent);
			if (itCurrent != labelSet.end())
			{
				segmentLabelSetIter itAbove = prev(itCurrent);
				segmentLabelSetIter itBelow = next(itCurrent);
				if ((itAbove != labelSet.end()) && (itBelow != labelSet.end()))
				{
					const LineFloat2D& lineAbove = *itAbove;
					const LineFloat2D& lineBelow = *itBelow;
					if (checkSegmentIntersection(lineAbove, lineBelow))
					{
						intersectingLines.insert(lineAbove);
						intersectingLines.insert(lineBelow);
					}
				}
			}
			labelSet.erase(lineCurrent);
		}
		sweepLineEventQueue.pop();
	}

	drawIntersectingLines(intersectingLines);
}

void ReadInputPointsFromFile(vector<PointFloat2D>& oInputPointsList)
{
	ifstream input("2dpointsdata.txt");
	bool dimCalculated = false; int dim = 0; string dataType = "";
	/*vector<PointFloat2D> listInputPointsFloat2D;*/
	for (string line; getline(input, line);) 
	{
		stringstream lineStream(line);

		if (dataType == "")
		{
			dataType = lineStream.str();
			continue;
		}
		if (dataType == "float")
		{
			vector<float> curentDataVec;
			curentDataVec.insert(curentDataVec.begin(), istream_iterator<float>(lineStream),
				istream_iterator<float>());
			if (!dimCalculated)
				dim = curentDataVec.size();

			assert(dim == curentDataVec.size());

			if (dim == 2)
			{
				PointFloat2D inputPoint(curentDataVec);
				oInputPointsList.push_back(inputPoint);
			}
		}
	}
}
void drawConvexHull()
{
	glClear(GL_COLOR_BUFFER_BIT);
	vector<PointFloat2D> listInputPointsFloat2D;
	ReadInputPointsFromFile(listInputPointsFloat2D);
	drawPoints(listInputPointsFloat2D);
	findAndDrawConvexHull(listInputPointsFloat2D);
}

void drawIntersection()
{
	glClear(GL_COLOR_BUFFER_BIT);
	vector<PointFloat2D> listInputPointsFloat2D;
	ReadInputPointsFromFile(listInputPointsFloat2D);
	drawLines(listInputPointsFloat2D);
	findIntersection(listInputPointsFloat2D);
}

/* Callback functions for GLUT */
void display(void)
{
	/* clear the screen to white */
	glClear(GL_COLOR_BUFFER_BIT);

	/* draw points */
	vector<PointFloat2D> listInputPointsFloat2D;
	ReadInputPointsFromFile(listInputPointsFloat2D);
	drawPoints(listInputPointsFloat2D);

	return;
}

/* Called when window is resized,
also when window is first created,
before the first call to display(). */
void reshape(int w, int h)
{
	/* save new screen dimensions */
	width = (GLdouble)w;
	height = (GLdouble)h;

	/* tell OpenGL to use the whole window for drawing */
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	/* do an orthographic parallel projection with the coordinate
	system set to first quadrant, limited by screen/window size */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, 0.0, height, -1.f, 1.f);

	return;
}

void kbd(unsigned char key, int x, int y)
{
	switch ((char)key) {
	case 'c':
	case 'C':
		drawConvexHull();
		break;
	case 's':
	case 'S':
		drawIntersection();
		break;
	case 'q':
	case 27:    /* ESC */
		glutDestroyWindow(wd);
		exit(0);
	default:
		break;
	}

	return;
}

int main(int argc, char *argv[])
{
	width = 1280.0;                 /* initial window width and height, */
	height = 800.0;                  /* within which we draw. */

	/* initialize GLUT, let it extract command-line
	GLUT options that you may provide
	- NOTE THE '&' BEFORE argc */
	glutInit(&argc, argv);

	/* specify the display to be single
	buffered and color as RGBA values */
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);

	/* set the initial window size */
	glutInitWindowSize((int)width, (int)height);

	/* create the window and store the handle to it */
	wd = glutCreateWindow("Convex Hull" /* title */);

	/* --- register callbacks with GLUT --- */

	/* register function to handle window resizes */
	glutReshapeFunc(reshape);

	/* register keyboard event processing function */
	glutKeyboardFunc(kbd);

	/* register function that draws in the window */
	glutDisplayFunc(display);

	/* init GL */
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glLineWidth(3.0);

	/* start the GLUT main loop */
	glutMainLoop();

	exit(0);
}