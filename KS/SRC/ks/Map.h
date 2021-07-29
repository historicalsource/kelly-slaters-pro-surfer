#ifndef MAPH
#define MAPH

#include "PathData.h"

#define MAP_DASHES

class MapData
{
public:
	
	// this is stored in Map, but most of the stuff is set in BeachFE
	struct MapBeach
	{
		bool desc_at_right;
//		vector3d loc;
		vector2d loc2d;		// for overlay
	};

	MapBeach* beaches;
	int* west_east_order; // beachdata indices in west to east order

private:
	int num_paths;
	PathData* paths;
	int max_pqs;

#ifdef MAP_DASHES
	PanelQuad4* copy;
	PanelQuad4* array;
#else
	PanelQuad* copy;
	PanelQuad* array;
#endif

	PathData* chosen_path;

public:
	MapData();
	~MapData();

	// this function is a basic, specific .ase file parser
	void Load(char* filename, PanelQuad* copy_pq);
	void Reload(PanelQuad* copy_pq);

	bool setPath(int beach_to, int beach_from);
	void DrawPath(float percent); // percent is between 0 & 1
};

#endif