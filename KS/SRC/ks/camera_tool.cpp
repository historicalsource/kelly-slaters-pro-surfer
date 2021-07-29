// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "menu.h"
#include "camera_tool.h"
#include "app.h"
#include "kshooks.h"


extern MenuSystem *menus;
//camera_list CameraList;
camera_list *CameraList=NULL;


MenuEntryFunction *entry_add_cam = NULL; //MenuEntryFunction( "Add Camera",   AddCamButton  );
MenuEntryTitle *camtool_title = NULL; //MenuEntryTitle("Camera Tool Menu");

//
// If the number of elements in camtool_entries changes then the
//   KSDBMENU_Init() routine should be updated to match.
//

pMenuEntry camtool_entries[2] = { NULL, NULL }; //&camtool_title, &entry_add_cam };
Menu *menu_inner_camtool = NULL; //Menu(NULL, 2, camtool_entries);
Submenu *menu_camtool = NULL; //Submenu ("Camera Tool Menu", &menu_inner_camtool);
MENUENTRYBUTTONFUNCTION(AddCamButton);

void CAMERA_TOOL_StaticInit( void )
{
	CameraList=NEW camera_list;
	entry_add_cam = NEW MenuEntryFunction( "Add Camera",   AddCamButton  );
	camtool_title = NEW MenuEntryTitle("Camera Tool Menu");
	camtool_entries[0] = camtool_title;
	camtool_entries[1] = entry_add_cam;
	menu_inner_camtool = NEW Menu(NULL, 2, camtool_entries);
	menu_camtool = NEW Submenu ("Camera Tool Menu", menu_inner_camtool);
}



void SetPO(camera *cam)
{
	if (cam)
	{
		entity * c1 = find_camera(entity_id("USER_CAM"));
		if (c1)
		{
			po me = c1->get_rel_po();
			cam->set_rel_po(me);
		}
	}
}

MENUENTRYBUTTONFUNCTION(AddCamButton)
{
	/*switch ( buttonid )
	{
		case MENUCMD_CROSS:
			app::inst()->get_game()->set_current_camera(find_camera(entity_id("KSDEBUG_CAM")));
			menus->CloseMenu();
			break;
	}*/
	return true;
}

bool ChangeCamMenuEntryFunction::ChangePOButton( int buttonid )
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			//app::inst()->get_game()->set_current_camera(find_camera(entity_id("BEACH_CAM")));
			menus->CloseMenu();
			break;
	}
	return true;
}

bool DeleteCamMenuEntryFunction::DeleteCamButton( int buttonid )
{
	switch ( buttonid )
	{
		case MENUCMD_CROSS:
			parent->deleted = true;
			menus->CloseMenu();
			menus->CloseMenu();
			break;
	}
	return true;
}

camMenuObj::camMenuObj(char *name, Menu *menu_parent, camera *cam)
{
	deleted = false;
	po_changed = false;
	p_dbg_cam = cam;
	parent = menu_parent;
	cam_title= NEW MenuEntryTitle(name);
	ent1 = NEW ChangeCamMenuEntryFunction( "Change PO", this );
	ent2 = NEW DeleteCamMenuEntryFunction("Delete Camera", this );
	inner_menu = NEW Menu(menu_parent);
	inner_menu->AddEntry(cam_title);
	inner_menu->AddEntry(ent1);
	inner_menu->AddEntry(ent2);
	cam_submenu = NEW Submenu(name, inner_menu);
	menu_parent->AddEntry(cam_submenu);
}

camMenuObj::~camMenuObj()
{
	inner_menu->DelEntry(cam_title);
	inner_menu->DelEntry(ent1);
	inner_menu->DelEntry(ent2);
	delete cam_title;
	delete ent1;
	delete ent2;
	parent->DelEntry(cam_submenu);
	delete inner_menu;
	delete cam_submenu;
}

camera_list::camera_list()
{
	 num_cams = 0; camlist_start = NULL; smenu = NULL; cam_menu = NULL;
	 //menu_main.AddEntry(&menu_camtool);
}

void camera_list::create_camera_list(int num)
{
	if (num > 0)
	{
		num_cams = num;
		camlist_start = NEW camera* [num];
		names = NEW char* [num];
	}
}

void camera_list::create_camera(int num, char *name, int type)
{
	if ((num >= 0) && (num < num_cams))
		camlist_start[num] = NEW camera(NULL, entity_id::make_entity_id(name));
}

camera_list::~camera_list()
{
	if (num_cams > 0)
	{
		for (int n = 0; n < num_cams; n++)
		{
			delete (camlist_start[n]);
			delete [] (names[n]);
		}

		delete[] camlist_start;
		delete[] names;
	}

	//menu_main.DelEntry(&menu_camtool);
}

void camera_list::Update(void)
{
	Node<camMenuObj> *node = dbmenu_list.GetFirst()->next;
	while (node != dbmenu_list.GetLast())
	{
		if (node->Obj->deleted == true)
		{
			Node<camMenuObj> *tnode = node;
			node = node->next;
			delete tnode->Obj;
			dbmenu_list.Remove(tnode);
		}
		else
			node = node->next;
	}
}

void camera_list::DestroyCameraList(void)
{
	if (num_cams > 0)
	{
		for (int n = 0; n < num_cams; n++)
		{
			delete (camlist_start[n]);
			delete [] (names[n]);
		}

		delete[] camlist_start;
		delete[] names;
	}

	dbmenu_list.DestroyList();
	delete cam_menu;
	menu_inner_camtool->DelEntry(smenu);
	delete smenu;
	smenu = NULL;
	cam_menu = NULL;
}


void Read_Camera_List(void)
{

	//printf("Hello I'm Read_Camera_List() and my memory allocation leaks\n");
	//printf("Hi Read_Camera_List()\n");

	// there is no ReleaseFile to match the following ReadFile

	#if 0
	ioFileBuf File;
	if (mReadFile("camera_list.csv", &File))
	#else
	nglFileBuf File;
	if (KSReadFile("camera_list.csv", &File, 1))
	#endif
	{
		int n = 0;
		char *s, *p = (char *) File.Buf;
		s = p;
		while ((*p != ',') && (*p != '\n') && (*p != '\r'))  //  until semicolon, carriage return, or line feed
		{
			p++;
			n++;
		}

		int num_cams = 0;
		for (int num = 0; num < n; num++, s++)
		{
			num_cams += ((int)pow(10, n - num - 1))*(*s - '0');
		}

		if (num_cams <= 0)
			return;

		CameraList->create_camera_list(num_cams);

		p++;
		while ((*p == '\n') || (*p == '\r'))
		{
			p++;
		}

		for (int cams = 0; cams < num_cams; cams++)
		{
			n = 0;
			s = p;
			while (*p != ',')
			{
				p++;
				n++;
			}

			CameraList->CreateName(cams, n+1);
			for (int i = 0; i < n; i++)
			{
				*(CameraList->GetName(cams) + i) = *s;
				s++;
			}

			*(CameraList->GetName(cams) + n) = 0;
			p++;

			CameraList->create_camera(cams, CameraList->GetName(cams));

			s = p;
			float x[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
			for (int i = 0; i < 6; i++, s = p)
			{
				n = 0;
				float pos = 1.0f;
				if (*s == '-')
				{
					pos = -1.0f;
					s++;
					p++;
				}

				bool decimal = false;	// was uninitialized (dc 01/29/02)
				int dec_place = -1;	// was uninitialized (dc 01/29/02)
				while (*p != ',')
				{
					if (*p == '.')
					{
						decimal = true;
						dec_place = n;
					}
					n++;
					p++;
				}

				for (int num = 0; num < n; num++, s++)
				{
					if (decimal && (num == dec_place))
						continue;
					else if (!decimal || (num < dec_place))
						x[i] += pow(10.0f, (decimal?dec_place:n) - num - 1)*(*s - '0');
					else
						x[i] += pow(0.1f, num - dec_place)*(*s - '0');
				}
				x[i] *= pos;
				p++;
			}

			while ((*p == '\n') || (*p == '\r'))
			{
				p++;
			}
		}
		KSReleaseFile(&File);
	}
}

void CreateDebugMenuTools(void)
{
	for (int cams = 0; cams < CameraList->GetNumCams(); cams++)
	{
		if (CameraList->smenu == NULL)
		{
			CameraList->cam_menu = NEW Menu(menu_inner_camtool);
			CameraList->smenu = NEW Submenu("Edit Cameras", CameraList->cam_menu);
			menu_inner_camtool->AddEntry(CameraList->smenu);
		}

		camMenuObj *Obj = NEW camMenuObj(CameraList->GetName(cams), CameraList->cam_menu, CameraList->camlist_start[cams]);
		CameraList->dbmenu_list.AddBack(Obj);
	}
}

void DestroyCameraList(void)
{
	CameraList->DestroyCameraList();
}

