#ifndef CAMERA_TOOL_H
#define CAMERA_TOOL_H

#include "global.h"
#include "game_info.h"
#include "wds.h"
#include "ksdbmenu.h"
#include "parser.h"
#include "camera.h"
#include "simple_list.h"

void Read_Camera_List(void);
void CreateDebugMenuTools(void);
void DestroyCameraList(void);

class camMenuObj;

class ChangeCamMenuEntryFunction : public MenuEntryLabel
{
	camMenuObj *parent;

	public:
	ChangeCamMenuEntryFunction( char *t, camMenuObj *p )
		: MenuEntryLabel( t )
	{ parent = p; }
	virtual ~ChangeCamMenuEntryFunction( ) {}

	bool ChangePOButton( int buttonid );

	virtual void OnButtonPress( int buttonid ) { ChangePOButton(buttonid); }
};

class DeleteCamMenuEntryFunction : public MenuEntryLabel
{
	camMenuObj *parent;

	public:
	DeleteCamMenuEntryFunction( char *t, camMenuObj *p )
		: MenuEntryLabel( t )
	{ parent = p; }
	virtual ~DeleteCamMenuEntryFunction( ) {}

	bool DeleteCamButton( int buttonid );

	virtual void OnButtonPress( int buttonid ) { DeleteCamButton(buttonid); }
};

class camMenuObj
{
public:
	camMenuObj(char *name, Menu *menu_parent, camera *cam);
	~camMenuObj();

	bool deleted;
	bool po_changed;

private:
	camera *p_dbg_cam;
	MenuEntryTitle *cam_title;
	ChangeCamMenuEntryFunction *ent1;
	DeleteCamMenuEntryFunction *ent2;
	Menu *inner_menu;
	Submenu *cam_submenu;
	Menu *parent;
};

class camera_list
{
public:
	void create_camera_list(int num);
	void create_camera(int num, char *name, int type = 0);
	camera_list();
	~camera_list();
	void CreateName(int n, int size) { names[n] = NEW char[size]; }
	char *GetName(int n) { return names[n]; }
	int GetNumCams(void) { return num_cams; }
	void Update(void);
	void DestroyCameraList(void);

	Submenu *smenu;
	Menu *cam_menu;

	camera **camlist_start;

	List<camMenuObj> dbmenu_list;

private:

	int num_cams;
	char **names;
};

#endif // CAMERA_TOOL_H
