

#include "global.h"
#include "BoardManager.h"
#include "surferdata.h"
#ifdef TARGET_GC
#include "ngl.h"
#endif

char g_BoardIFLs[NUM_IFL_FILES][400];
char IFLFileNames[NUM_IFL_FILES][12] =
{
  "brdtop",
  "brdbot",
  "fnbl",
  "fnbld",
  "fnwhd"
};

int numTex = 0;
void BOARD_GenAndLoadBoardIFLs(int surfer_index, int board_index, int hero1,  int surfer_index2, int board_index2, int hero2)
{
  char iflName[20];
  bool pers;
  for (int j=0; j < NUM_IFL_FILES; j++)
  {
    sprintf(iflName, "%s.ifl", IFLFileNames[j]);
    stringx texpath[2];
    texpath[0] = texpath[1] = "";
    memset(g_BoardIFLs [j], '\0', 256);
      char line[80];
      if (board_index < MAX_BOARDS)
      {
				
        pers = g_game_ptr->GetUsingPersonalitySuit(hero1);
        if (!pers)
        {
					if (stricmp(SurferDataArray[surfer_index].board_ent_name, "BOARD")==0)
					{
						if (texpath[0] == "")
							texpath[0] =stringx("BOARDS\\") + stringx(SurferDataArray[surfer_index].name) + stringx("\\TEXTURES\\");
						sprintf(line, "%s_%d_%s.tga\n\r", SurferDataArray[surfer_index].abbr, board_index, IFLFileNames[j]);
						strcat(g_BoardIFLs[j], line);
					}

        }
        else
        {
					if (stricmp(SurferDataArray[surfer_index].p_board_name, "BOARD")==0)
					{

						if (texpath[0] == "")
							texpath[0] =stringx("BOARDS\\PERSONALITY") + stringx(SurferDataArray[surfer_index].abbr) + stringx("\\TEXTURES\\");
						sprintf(line, "%s_P_%s.tga\n\r", SurferDataArray[surfer_index].abbr, IFLFileNames[j]);
						strcat(g_BoardIFLs[j], line);
					}
        }
      }
			else   if (board_index >= MAX_BOARDS) // location
			{

				if (texpath[0] == "")
					texpath[0] =stringx("BOARDS\\BEACHES\\TEXTURES\\");
				sprintf(line, "%s_%s.tga\n\r", g_game_ptr->get_beach_board_name(board_index - MAX_BOARDS).c_str(), IFLFileNames[j]);
				strcat(g_BoardIFLs[j], line);
			}
      if (	board_index2 < MAX_BOARDS)
      {
        pers = g_game_ptr->GetUsingPersonalitySuit(hero2);
        if (!pers)
        {
					if (stricmp(SurferDataArray[surfer_index2].board_ent_name, "BOARD")==0)
					{

						if (texpath[1] == "")
						{
							texpath[1] =stringx("BOARDS\\") + stringx(SurferDataArray[surfer_index2].name) + stringx("\\TEXTURES\\");
						}
						sprintf(line, "%s_%d_%s.tga\n\r", SurferDataArray[surfer_index2].abbr, board_index2, IFLFileNames[j]);
						strcat(g_BoardIFLs[j], line);
					}
        }
        else
        {
					if (stricmp(SurferDataArray[surfer_index2].p_board_name, "BOARD")==0)
					{
						if (texpath[1] == "")
						{
							texpath[1] =stringx("BOARDS\\PERSONALITY") + stringx(SurferDataArray[surfer_index2].abbr) + stringx("\\TEXTURES\\");
						}
						sprintf(line, "%s_P_%s.tga\n\r", SurferDataArray[surfer_index2].abbr, IFLFileNames[j]);
						strcat(g_BoardIFLs[j], line);
					}
        }
      }	
			else  if (board_index2 >= MAX_BOARDS)// location
			{
				if (texpath[0] == "")
					texpath[0] =stringx("BOARDS\\BEACHES\\TEXTURES\\");
				sprintf(line, "%s_%s.tga\n\r", g_game_ptr->get_beach_board_name(board_index2 - MAX_BOARDS).c_str(), IFLFileNames[j]);
				strcat(g_BoardIFLs[j], line);
			}
    
    nglSetTexturePath("");
    g_file_finder->push_path_back(texpath[0].c_str());
    g_file_finder->push_path_back(texpath[1].c_str());
    nglLoadTextureInPlace(IFLFileNames[j], NGLTEX_IFL, (void *)g_BoardIFLs[j], strlen(g_BoardIFLs[j]));
    g_file_finder->pop_path_back();
    g_file_finder->pop_path_back();
  }
}



void BOARD_GenIFLFileLine(char *whileIFL, int surfer_index, bool pers, int whichLine)
{

}
// Change this to work on the filenames alone!
void BOARD_CompletelyUnloadIFLFiles(/*entity *board*/)
{
  nglTexture *tex;
  /*if (board)
  {
    nglMesh *m = nglGetFirstMeshInFile((board->fileName + stringx("_lo")).c_str());
    if (m)
    {
      for (u_int j=0; j < m->NSections; j++)
      {
        while((tex = nglGetTexture(m->Sections[j].Material->MapName)) != NULL)
          nglReleaseTexture(tex);
      }
    }

    m = nglGetFirstMeshInFile(board->fileName.c_str());
    if (m)
    {
      for (u_int j=0; j < m->NSections; j++)
      {
        while((tex = nglGetTexture(m->Sections[j].Material->MapName)) != NULL)
          nglReleaseTexture(tex);
      }
    }
  }*/
  for (int i=0; i < NUM_IFL_FILES; i++)
  {
    while((tex = nglGetTexture(nglFixedString(IFLFileNames[i]))) != NULL)
      nglReleaseTexture(tex);
  }
}
void BOARD_UnloadMesh(entity *board)
{
	if (!board)
		return;
  while (nglGetFirstMeshInFile((board->fileName + stringx("_lo")).c_str()))
	nglReleaseMeshFile((board->fileName + stringx("_lo")).c_str());

  ((conglomerate*)board)->get_member("BOARD")->set_lores_mesh((nglMesh *)0x0);
  
  
	  while (nglGetFirstMeshInFile((board->fileName).c_str()))
    nglReleaseMeshFile((board->fileName).c_str());
  ((conglomerate*)board)->get_member("BOARD")->set_mesh((nglMesh *)0x0);

  
}

void BOARD_LoadMesh(entity *board, int hero_num)
{
  if (!g_game_ptr->GetUsingPersonalitySuit(hero_num))
  {
		
    if (g_game_ptr->GetBoardIdx(hero_num) < MAX_BOARDS && stricmp(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].board_ent_name, "BOARD") != 0)
    {
			board->fileName = SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].board_ent_name;
      stringx meshpath = stringx("CHARACTERS\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].name) + stringx("\\ENTITIES\\");;
			stringx texpath  =  stringx("BOARDS\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].name) + stringx("\\TEXTURES\\");
      nglSetMeshPath(meshpath.c_str());
			nglSetTexturePath(texpath.c_str());
    }
    else
    {
			board->fileName = "BOARD";
      nglSetMeshPath("ITEMS\\BOARD\\ENTITIES\\");
			nglSetTexturePath("ITEMS\\BOARD\\TEXTURES\\");
    }
  }
  else
  {


    if (g_game_ptr->GetBoardIdx(hero_num) < MAX_BOARDS && stricmp(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].p_board_name, "BOARD") != 0)
    {
			board->fileName = SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].p_board_name;
      nglSetMeshPath((stringx("CHARACTERS\\PERSONALITY") + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr + "\\ENTITIES\\").c_str());
			nglSetTexturePath((stringx("BOARDS\\PERSONALITY") + SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].abbr + "\\TEXTURES\\").c_str());
    }
    else
    {
			board->fileName = "BOARD";
      nglSetMeshPath("ITEMS\\BOARD\\ENTITIES\\");
			nglSetMeshPath("ITEMS\\BOARD\\TEXTURES\\");
    }
  }
  nglLoadMeshFile((board->fileName + stringx("_lo")).c_str());
  ((conglomerate*)board)->get_member("BOARD")->set_lores_mesh(nglGetFirstMeshInFile((board->fileName + stringx("_lo")).c_str()));
	
  nglLoadMeshFile((board->fileName).c_str());
  ((conglomerate*)board)->get_member("BOARD")->set_mesh(nglGetFirstMeshInFile(board->fileName.c_str()));


	// I don't know why this is done here, it's also in kellyslater_controller::kellyslater_controller() but I need to disable this for personality suits - leo
	// This is changed so that now we only have the entries for the two sets of boards
  if (!g_game_ptr->GetUsingPersonalitySuit(hero_num) || !stricmp(SurferDataArray[g_game_ptr->GetSurferIdx(hero_num)].p_board_name, "board"))
	  ((conglomerate*)board)->get_member("BOARD")->SetTextureFrame(hero_num);
}

void BOARD_ReloadTextures(entity *board, entity *board2, int surfer_index, int board_index, int surfer_index2, int board_index2)
{
// This code breaks nglGC for some reason
// It fires an assert in nglSetupTevs:
// assert( Params->TextureFrame < Map->NFrames )
//#ifndef TARGET_GC
  BOARD_CompletelyUnloadIFLFiles();
  numTex = 0;
  if (surfer_index2 >=	 0)
  {
    BOARD_UnloadMesh(board);
    BOARD_UnloadMesh(board2);

    BOARD_GenAndLoadBoardIFLs(surfer_index, board_index, 0, surfer_index2,board_index2, 1);
    BOARD_LoadMesh(board, 0);
    BOARD_LoadMesh(board2, 1);
    
  }
  else
  {
    BOARD_UnloadMesh(board);
    BOARD_GenAndLoadBoardIFLs(surfer_index,board_index, 0);
    BOARD_LoadMesh(board, 0);	

  }
//#endif TARGET_GC
}
