#ifndef PATHDATAH
#define PATHDATAH


#define MAX_POINTS_PER_PATH 70

struct myVector2d
{
  float x;
  float y;
};
struct PathData
{
	//char name[30];
	int beach1;
	int beach2;
	int num_points;
	myVector2d points[MAX_POINTS_PER_PATH];
	//PathData* next;
};

#endif