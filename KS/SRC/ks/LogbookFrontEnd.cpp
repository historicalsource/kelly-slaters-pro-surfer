// LogbookFrontEnd.cpp

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"
#if _XBOX
#include "xbglobals.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "LogbookFrontEnd.h"

LogbookFrontEnd::LogbookFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
{
	cons(s, man, p, pf_name);
	sys = (GraphicalMenuSystem*) system;

	FEMenuEntry* feme = NEW FEMenuEntry("", this);
	Add(feme);

//	color32 col = manager->col_bio;
	color32 col = color32(50, 50, 50, 255);
	for(int i=0; i<max_notes; i++)
	{
		notes[i] = NEW BoxText(&manager->font_body, "", 0, 0, 0, 0.9f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, col, 20);
		dates[i] = NEW TextString(&manager->font_bold_old, "", 0, 0, 0, 0.9f, Font::HORIZJUST_LEFT, Font::VERTJUST_CENTER, col);
	}
}

LogbookFrontEnd::~LogbookFrontEnd()
{
	for(int i=0; i<max_notes; i++)
	{
		delete notes[i];
		delete dates[i];
	}
}

void LogbookFrontEnd::Load()
{
	PanelFile* pf = &((AccompFrontEnd*) sys->menus[GraphicalMenuSystem::AccompMenu])->panel;
	book = pf->GetPointer("sb_book");
//	ReadNotesFromFile();

	// set dates
//	stringx months[] = { "MAY", "JUNE", "JULY", "AUGUST" };
	stringx months[] = { "May", "June", "July", "August" };
	int max_days[] = { 31, 30, 31, 31 };
	int month = 0;
	int day = 10;
	for(int i=0; i<max_notes; i++)
	{
		dates[i]->changeText(months[month]+" "+stringx(day+1));

		day += 3;
		if(day >= max_days[month])
		{
			day = 0;
			month++;
		}
	}
}

void LogbookFrontEnd::Draw()
{
	FEMultiMenu::Draw();
	for(int i=0; i<max_notes; i++)
	{
		if(note_page_num[i] == cur_spread*2 || note_page_num[i] == cur_spread*2+1)
		{
			notes[i]->Draw();
			dates[i]->Draw();
		}
	}
	book->Draw(0);
}

void LogbookFrontEnd::OnActivate()
{
	ReadNotesFromFile();
	FEMultiMenu::OnActivate();
	cur_spread = 0;
	UpdateUnlockedNotes();

	manager->helpbar->Reset();
	manager->helpbar->AddArrowH(ksGlobalTextArray[GT_FE_MENU_SWITCH]);
	manager->helpbar->RemoveX();
	manager->helpbar->Reformat();
}

void LogbookFrontEnd::UpdateUnlockedNotes()
{
	int offset = start_y;
	bool left_side = true;
	int page_num = 0;
	for(int i=0; i<LEVEL_LAST; i++)
	{
		// if note_page_num is less than 0, then it's invalid
		note_page_num[i] = -1;
		int l = g_career->GetLevelCompletedAt(i);
		if(l != -1 && note_body[l] != "")
		{
			dates[i]->changePos(left_side ? date_l_x : date_r_x, offset);
			offset += y_diff;
			notes[i]->changePos(left_side ? date_l_x : date_r_x, offset);
			notes[i]->changeText(note_body[l]);
			int n = notes[i]->makeBox(edge_l_x - date_l_x, 200);
			offset += y_diff*n;

			if(offset > end_y)
			{
				page_num++;
				left_side = !left_side;
				offset = start_y;
				dates[i]->changePos(left_side ? date_l_x : date_r_x, offset);
				offset += y_diff;
				notes[i]->changePos(left_side ? date_l_x : date_r_x, offset);
				offset += y_diff*n;
			}
			offset += y_extra_spacing;
			note_page_num[i] = page_num;
		}
	}
	max_spread = page_num/2;
}

void LogbookFrontEnd::ReadNotesFromFile()
{
	for(int i=0; i<max_notes; i++)
		note_body[i] = "";

	stringx filename = "";
	if(g_career->GetSurferIdx() == SURFER_KELLY_SLATER)
		filename = "levels\\frontend\\overlays\\logbook_ks";
	else filename = "levels\\frontend\\overlays\\logbook";

	if(ksGlobalTextLanguage == LANGUAGE_FRENCH)
		filename += "_fr.txt";
	else if(ksGlobalTextLanguage == LANGUAGE_GERMAN)
		filename += "_ge.txt";
	else filename += ".txt";
	
	nglFileBuf file;
	file.Buf=NULL; file.Size=0;
	KSReadFile(filename.data(), &file, 1);
	unsigned char* buffer = (unsigned char*) file.Buf;
	if(!buffer)
	{
		KSReleaseFile(&file);
		return;
	}

	buffer[file.Size] = '\0';	// terminate string, using the extra byte allocated in wds_readfile (dc 05/19/02)

	unsigned int cur_char_index = 0;
	int index_within_line = 0;
	int note_index = -1;
	char current_line[max_line_size];

	while(cur_char_index < file.Size && buffer[cur_char_index] != '\x04')
	{
		while(buffer[cur_char_index] != '\n' && buffer[cur_char_index] != '\r' && buffer[cur_char_index] != '\x04')
		{
			if(index_within_line < max_line_size)
				current_line[index_within_line++] = buffer[cur_char_index++];
			else
			{
				index_within_line++;
				cur_char_index++;
			}
		}

		if(buffer[cur_char_index] == '\r' && buffer[cur_char_index+1] == '\n')
			cur_char_index++;	// skip over the \n as well
		cur_char_index++;

		if(index_within_line >= max_line_size)
			current_line[max_line_size-1] = '\0';
		else
			current_line[index_within_line] = '\0';

		if(note_index == -1)
			note_index = atoi(current_line);
		else
		{
			assert(note_index >= 0 && note_index < max_notes);
			note_body[note_index] = stringx(current_line);
//			note_body[note_index].to_upper();
			note_index = -1;
		}

		current_line[0] = '\0';
		index_within_line = 0;
	}

	KSReleaseFile(&file);
}

void LogbookFrontEnd::OnTriangle(int c)
{
	SoundScriptManager::inst()->playEvent(SS_FE_BACK);
	FEMultiMenu::OnTriangle(c);
	sys->MakeActive(GraphicalMenuSystem::ExtrasMenu);
}