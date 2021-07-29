#include "global.h"
#include "map.h"
#include "PathData.h"
#ifdef TARGET_GC
#include "gc_algebra.h"
#endif
/****************************** MapData **********************************/

MapData::MapData()
{
	num_paths = 0;
	paths = NULL;
}

MapData::~MapData()
{
	delete[] array;
	delete[] west_east_order;
	delete[] beaches;
	delete[] paths;
#ifdef MAP_DASHES
	delete copy;
#endif
}


void MapData::Load(char* filename, PanelQuad* copy_pq)
{
	nglFileBuf file;
	file.Buf = NULL;
	file.Size = 0;
	KSReadFile(filename, &file, 1);
	char* buffer = (char*) file.Buf;
	if(!buffer)
	{
		KSReleaseFile(&file);
		assert(0);
	}
	num_paths =  file.Size/sizeof(PathData);
#ifdef DEBUG
	assert ((float)num_paths == (float)file.Size/(float)sizeof(PathData));
#endif
	paths = NEW PathData[num_paths];

	memcpy(paths, file.Buf, file.Size);
	KSReleaseFile(&file);

	west_east_order = NEW int[BEACH_LAST];
	beaches = NEW MapBeach[BEACH_LAST];
	for(int i=0; i<BEACH_LAST; i++)
	{
//		beaches[i].loc = vector3d(0, 0, 0);
		beaches[i].loc2d = vector2d(0, 0);
		beaches[i].desc_at_right = true;
	}

	max_pqs = 0;
	PathData* p = paths;
	for(int i=0; i<num_paths; i++)
	{
		#ifdef TARGET_GC
			// fixup endian funness
			fixup((unsigned char *) &p[i].beach1,sizeof(int));
			fixup((unsigned char *) &p[i].beach2,sizeof(int));
			fixup((unsigned char *) &p[i].num_points,sizeof(int));
			for (int j=0; j<MAX_POINTS_PER_PATH; j++)
			{
				fixup((unsigned char *) &p[i].points[j].x,sizeof(float));
				fixup((unsigned char *) &p[i].points[j].y,sizeof(float));
			}
		#endif
	  if(p[i].num_points > max_pqs) max_pqs = p[i].num_points;
	}

#ifdef MAP_DASHES
	copy = NEW PanelQuad4("path");
	copy->Init(0, 3, 3, 0, 0, 0, 7, 7, 2.0f, 2.0f, 0.8f, 2.0f, 0);
	array = NEW PanelQuad4[max_pqs];
	for(int i=0; i<max_pqs; i++) array[i] = PanelQuad4(*copy);
#else
	copy = copy_pq;
	array = NEW PanelQuad[max_pqs-2];
	for(int i=0; i<max_pqs-2; i++) array[i] = PanelQuad(*copy);
#endif

	chosen_path = NULL;
}

void MapData::Reload(PanelQuad* copy_pq)
{
#ifndef MAP_DASHES
	for(int i=0; i<max_pqs; i++)
		array[i].setTexture(copy->GetTexture());
#endif
}

bool MapData::setPath(int to, int from)
{
	chosen_path = NULL;
	PathData* p = paths;
	for(int i=0; i<num_paths; i++)
	{
		if(p[i].beach2 == to && p[i].beach1 == from)
		{
			chosen_path = &p[i];
			break;
		}
	}

	if(chosen_path)
	{
#ifndef MAP_DASHES
		for(int i=0; i<chosen_path->num_points; i++)
		{
			array[i].SetCenterX(chosen_path->points[i].x);
			array[i].SetCenterY(chosen_path->points[i].y);
			array[i].SetLayer(0);
		}
#else MAP_DASHES
		for(int i=0; i<chosen_path->num_points-2; i++)
		{
			array[i].SetCenterX(chosen_path->points[i+1].x);
			array[i].SetCenterY(chosen_path->points[i+1].y);
			array[i].SetLayer(0);

			float alpha, beta, theta, gamma, omega;
			vector2d prev = vector2d(chosen_path->points[i].x, chosen_path->points[i].y);
			vector2d cur = vector2d(chosen_path->points[i+1].x, chosen_path->points[i+1].y);
			vector2d next = vector2d(chosen_path->points[i+2].x, chosen_path->points[i+2].y);

			// adjust for going from one edge of the map to the other
			if((cur.x - next.x > 200) || (next.x - cur.x > 200))
			{
				next.x = 2*cur.x - prev.x;
				next.y = 2*cur.y - prev.y;
			}
			if((cur.x - prev.x > 200) || (prev.x - cur.x > 200))
			{
				prev.x = 2*cur.x - next.x;
				prev.y = 2*cur.y - next.y;
			}

			vector2d qp = prev - cur;
			vector2d qr = next - cur;

			float dot_result = dot(vector2d(0, -1), qp) / qp.length();
			if(dot_result < -1.0f) dot_result = -1.0f;
			if(dot_result > 1.0f) dot_result = 1.0f;
			omega = acosf(dot_result);

			dot_result = dot(vector2d(0, -1), qr) / qr.length();
			if(dot_result < -1.0f) dot_result = -1.0f;
			if(dot_result > 1.0f) dot_result = 1.0f;
			gamma = acosf(dot_result);

			if(qp.x > 0) omega = -omega;
			if(qr.x < 0) gamma = -gamma;
			alpha = gamma + omega;
			if(alpha < 0) alpha += 6.28f;

			theta = (3.14f - alpha)/2.0f;
			beta = gamma + theta;

			array[i].RotateOnce(cur.x, cur.y, beta);
		}
#endif
	}
	else nglPrintf("Missing Path from %d to %d\n", from, to);
	return (chosen_path != NULL);
}

void MapData::DrawPath(float percent)
{
	if(!chosen_path) return;
	int stop = (int)(percent*chosen_path->num_points+1);

#ifdef MAP_DASHES
	for(int i=0; i<chosen_path->num_points-2; i++)
		if(i >= stop) break;
		else array[i].Draw(0);
#else
	for(int i=0; i<chosen_path->num_points; i++)
		if(i >= stop) break;
		else array[i].Draw(0);
#endif
}

