
#ifndef BOARDMANAGERH
#define BOARDMANAGERH
#include "entity.h"
#include "ngl.h"

#define NUM_IFL_FILES 5
extern char g_BoardIFLs[NUM_IFL_FILES][400];
extern char IFLFileNames[NUM_IFL_FILES][12];

void        BOARD_GenAndLoadBoardIFLs(int surfer_index, int board_index, int hero1,  int surfer_index2=-999, int board_index2=-999, int hero2=-1);
void        BOARD_GenIFLFile(char *data, int surfer_index, bool pers, int which);
void        BOARD_CompletelyUnloadIFLFiles(/*entity *board*/);
entity*     BOARD_Load(int surfer_index, bool pers);
void BOARD_UnloadMesh(entity *board);
void BOARD_ReloadTextures(entity *board, entity *board2, int surfer_index, int board_index, int surfer_index2=-999, int board_index2=-999);

void        BOARD_UpdateTextures(entity *board);
#endif