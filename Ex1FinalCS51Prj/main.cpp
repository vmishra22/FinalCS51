#include "Point.h"
#include "Line.h"
#include "Event.h"
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

#define NENDS 2           /* number of end "points" to draw */

GLdouble width, height;   /* window width and height */
int wd;                   /* GLUT window handle */

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

	for (vector<PointFloat2D>::iterator it = iListPoints.begin(); it != iListPoints.end(); it = next(it,2))
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
	for (vector<PointFloat2D>::iterator it = iListInputPointsFloat2D.begin(); it != iListInputPointsFloat2D.end(); it = next(it, 2))
	{
		PointFloat2D leftPoint = *it;
		PointFloat2D rightPoint = *(next(it));
		LineFloat2D lineSegment = LineFloat2D(leftPoint, rightPoint);

		oEventVec.push_back(EventFloat2D(leftPoint, lineSegment, true));
		oEventVec.push_back(EventFloat2D(rightPoint, lineSegment, false));
	}
}

//Minimum X element, if there is a tie then lesser Y value element is before the other one.
bool compare_MinX(const PointFloat2D& a, const PointFloat2D& b)
{
	bool retVal = false;
	if ((fabs(a[0] - b[0]) < EPS) && (fabs(a[0] - b[0]) >= 0) && ((a[1] - b[1]) < EPS))
		retVal = true;
	else if ((a[0] - b[0] < 0) && (fabs(a[0] - b[0]) > EPS))
		retVal = true;
	else
		retVal = false;
	return retVal;
}

class eventsComparator
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

void findIntersection(vector<PointFloat2D>& iListPoints)
{
	vector<EventFloat2D> eventVector;
	createEvents(iListPoints, eventVector);

	//Create a Priority queue of min-heap for X-coordinate of event
	typedef priority_queue<EventFloat2D, vector<EventFloat2D>, eventsComparator> eventsPQ;
	eventsPQ(eventsComparator(), eventVector);

}

void GetAndDrawPoints()
{
	ifstream input("2dpointsdata.txt");
	bool dimCalculated = false; int dim = 0; string dataType = "";
	vector<PointFloat2D> listInputPointsFloat2D;
	for (string line; getline(input, line);) {
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
				listInputPointsFloat2D.push_back(inputPoint);
			}
				
		}		
	}
	//drawLines(listInputPointsFloat2D);
	//findIntersection(listInputPointsFloat2D);
	
	drawPoints(listInputPointsFloat2D);
	findAndDrawConvexHull(listInputPointsFloat2D);
}


/* Callback functions for GLUT */

/* Draw the window - this is where all the GL actions are */
void display(void)
{
	/* clear the screen to white */
	glClear(GL_COLOR_BUFFER_BIT);

	/* draw points */
	GetAndDrawPoints();

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