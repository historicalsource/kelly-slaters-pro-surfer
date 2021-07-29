// Accomplishments Front End Screen

#ifndef ACCOMPFRONTEND_H
#define ACCOMPFRONTEND_H


#include "FEMenu.h"
#include "CompressedPhoto.h"
class GraphicalMenuSystem;

class AccompFrontEnd : public FEMultiMenu
{
private:
	int num_photos;
	int num_pages;
	int cur_page;
	static const int MAX_PAGES = 10;
	int beach_index[MAX_PAGES];
	int level_index[MAX_PAGES];

	// this is used to associate a magazine cover with a particular beach
	int mag_cover_beach_index[BEACH_LAST];
	bool picture_exists[MAX_PAGES];
	static const int num_covers = 8;
	PanelQuad* mag_covers[num_covers][2];	// 8 different covers on two sides
	PanelQuad* mag_photos[2];				// 2 sides
	PanelQuad* mag_bright_cover[2];			// 2 sides
	PanelQuad* mag_dark_cover[2];
	PanelQuad* mag_shadow[2];
	PanelQuad* mag_corners[4][2];			// 4 corners, 2 sides
	PanelQuad* pol_corners[4][2];
	PanelQuad* pol_covers[2];
	PanelQuad* pol_photos[2];
	nglTexture *tempTex[2];
	nglTexture *noImage;

	TextString* beach[2];					// 2 sides
public:
	AccompFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name);
	virtual ~AccompFrontEnd();
	virtual void Load();
	virtual void Select(int entry_index) {}
	virtual void Draw();
	virtual void OnActivate();
	virtual void OnUp(int c) {}
	virtual void OnDown(int c) {}
	virtual void OnLeft(int c);
	virtual void OnRight(int c);
	virtual void OnTriangle(int c);
	
private:
	void UpdatePage(bool right_page);
	void UpdatePhotos();
	void SetPQIndices();
	void SwitchPages(bool right);
};

#endif