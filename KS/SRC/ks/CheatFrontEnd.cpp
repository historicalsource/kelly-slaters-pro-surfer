// CheatFrontEnd.cpp

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "FrontEndManager.h"
#include "CheatFrontEnd.h"

#if defined(TARGET_XBOX)
#include "osdevopts.h"
#include "MusicMan.h"
#include "ksnsl.h"
#endif /* TARGET_XBOX JIV DEBUG */


static bool toggle_cheats_entryAdded=false;
CheatFrontEnd::CheatFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
{
	cons(s, man, p, pf_name);
	sys = (GraphicalMenuSystem*) s;

	flags |= FEMENU_USE_SCALE;
	flags &= ~FEMENU_NO_FLASHING;
	scale = scale_high = PHONE_TEXT_SIZE*1.18f;

	pq_indices_set = false; // the pq indices haven't been set yet

	color = PHONE_TEXT_COLOR;
	color_high = PHONE_TEXT_COLOR_HI;
	color_high_alt = PHONE_TEXT_COLOR;

	// Make the "enter a new cheat code" entry at the top of the menu
	new_cheat_entry = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_ADD_NEW_CHEAT], this, false, &frontendmanager.font_bold_old);
	new_cheat_entry->SetPos(PHONE_SCREEN_LEFT, PHONE_SCREEN_TOP);
	new_cheat_entry->SetHJustify(Font::HORIZJUST_LEFT);
	new_cheat_entry->SetVJustify(Font::VERTJUST_TOP);
	Add(new_cheat_entry);

	// Make the "toggle cheats" menu entry but don't add it until there are unlocked cheats
	// This was a leak!  I put in a work around, but it isn't added (and not deleted) if the menu isn't
	// activated
	// KES 
	toggle_cheats_entry = NEW FEMenuEntry(ksGlobalTextArray[GT_FE_MENU_TOGGLE_CHEAT], this, false, &frontendmanager.font_bold_old);
	toggle_cheats_entry->SetPos(PHONE_SCREEN_LEFT, PHONE_SCREEN_TOP + PHONE_MAIN_MENU_VERT_SPACING);
	toggle_cheats_entry->SetHJustify(Font::HORIZJUST_LEFT);
	toggle_cheats_entry->SetVJustify(Font::VERTJUST_TOP);
	toggle_cheats_entry->down = toggle_cheats_entry->up = new_cheat_entry;
	toggle_cheats_entryAdded=false;
	// Create the cheat code listing sub-menu
	code_menu  = NEW CheatCodeMenu(s, man, p, pf_name);
	enter_code = NEW EnterCheatMenu(s, man, p, pf_name);
	AddSubmenu(code_menu);
	AddSubmenu(enter_code);
}

CheatFrontEnd::~CheatFrontEnd()
{
	delete code_menu;
	delete enter_code;
	if (!toggle_cheats_entryAdded)
		delete toggle_cheats_entry;
}

void CheatFrontEnd::Load()
{
	FEMultiMenu::Load();
	enter_code->Load();
	code_menu->Load();
	TurnOnPhone(false);
}

void CheatFrontEnd::Draw()
{
	if(active)
		active->Draw();
	else
		FEMultiMenu::Draw();
}

void CheatFrontEnd::Select(int entry_index)
{
	if(active)
		active->Select(entry_index);
	else
	{
		switch(entry_index)
		{
		case CHEAT_MENU_ENTER_CODE:
			MakeActive(enter_code);
			break;

		case CHEAT_MENU_TOGGLE_CHEAT:
			MakeActive(code_menu);
			break;
		}
	}
}

void CheatFrontEnd::Select()
{
	assert(highlighted);
	Select(highlighted->entry_num);
}

void CheatFrontEnd::OnTriangle(int c)
{
	if(active)
		active->OnTriangle(c);
	else 
	{
		TurnOnPhone(false);
		sys->MakeActive(GraphicalMenuSystem::ExtrasMenu);
	}
}

void CheatFrontEnd::OnActivate()
{
	bool cheat_available = false;

	TurnOnPhone(true);

	// Check for whether there are currently any cheats unlocked
	for(int i = 0; i < CHEAT_LAST; i++)
		if(globalCareerData.isCheatUnlocked(i))
			cheat_available = true;

	if(entries->next == NULL && cheat_available)
	{
		// Activate the "toggle cheats" entry appear
		toggle_cheats_entryAdded = true;
		Add(toggle_cheats_entry);
		new_cheat_entry->down = new_cheat_entry->up = toggle_cheats_entry;
	}

	FEMultiMenu::OnActivate();
}

void CheatFrontEnd::OnUp(int c)
{
	if(active) 
		active->OnUp(c);
	else 
	{
		if (this->highlighted->up)
			SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
		else
			SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
		FEMultiMenu::OnUp(c);
	}
}

void CheatFrontEnd::OnDown(int c)
{
	if(active) 
		active->OnDown(c);
	else 
	{
		if (this->highlighted->down)
			SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
		else
			SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
		FEMultiMenu::OnDown(c);
	}
}

void CheatFrontEnd::OnLeft(int c)
{
	if(active) 
		active->OnLeft(c);
	else 
		FEMultiMenu::OnLeft(c);
}

void CheatFrontEnd::OnRight(int c)
{
	if(active) 
		active->OnRight(c);
	else 
		FEMultiMenu::OnRight(c);
}

void CheatFrontEnd::SetPQIndices()
{
	stringx number_name;

	if(!pq_indices_set)
	{
		arrow_up[0] = GetPointer("btn_up_off");
		arrow_up[1] = GetPointer("btn_up_on");
		arrow_down[0] = GetPointer("btn_down_off");
		arrow_down[1] = GetPointer("btn_down_on");
		cellphone = GetPointer("cellphone");

		for(int i = 0; i < 10; i++)
		{
			number_name.printf("key_%d_yellow", i);
			numbers[i] = GetPointer(number_name.c_str());
			number_name.printf("key_%d_green", i);
			numbers_hi[i] = GetPointer(number_name.c_str());
		}

		pq_indices_set = true;
	}
}

void CheatFrontEnd::TurnOnPhone(bool on)
{
	SetPQIndices();

	//Turn off the cell phone, etc
	arrow_up[0]->TurnOn(false);
	arrow_up[1]->TurnOn(false);
	arrow_down[0]->TurnOn(false);
	arrow_down[1]->TurnOn(false);
	cellphone->TurnOn(on);

	for(int i = 0; i < 10; i++)
	{
		numbers[i]->TurnOn(on);
		numbers_hi[i]->TurnOn(false);
	}
}

	
///////////////////////////////////////////////////////////////////////////////////////
// CheatCodeMenu
//
// This is the menu listing all the currently unlocked cheat codes
///////////////////////////////////////////////////////////////////////////////////////

CheatCodeMenu::CheatCodeMenu(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
{
	cons(s, man, p, pf_name);
	sys = (GraphicalMenuSystem*) s;

	flags |= FEMENU_USE_SCALE;
	flags &= ~FEMENU_NO_FLASHING;
	scale = scale_high = PHONE_TOGGLE_TEXT_SIZE*1.5f;
	color = PHONE_TEXT_COLOR;
	color_high = PHONE_TEXT_COLOR_HI;
	color_high_alt = PHONE_TEXT_COLOR;

	next_up = next_down = -1;
}

CheatCodeMenu::~CheatCodeMenu()
{
	for(int i=0; i < MAX_CHEATS_PER_SCREEN; i++)
	{
		delete toggle_state[i];
	}
}

void CheatCodeMenu::Draw()
{
	for(int i=0; i < MAX_CHEATS_PER_SCREEN; i++)
	{
		toggle_state[i]->Draw();
	}
	FEMultiMenu::Draw();
}

void CheatCodeMenu::Load()
{
	FEMultiMenu::Load();
	SetPQIndices();

	// Initialize all the cheat menu items to be NULL
	for(int i = 0; i < MAX_CHEATS_PER_SCREEN; i++)
	{
		cheats[i] = NULL;
		menu_entry_cheat_index[i] = -1;
		toggle_state[i] = NEW TextString(&frontendmanager.font_body, "", PHONE_SCREEN_RIGHT, 
		                                 PHONE_SCREEN_TOP + PHONE_TOGLE_TOP_OFFSET + PHONE_LIST_VERT_SPACING * i, 
		                                 0, PHONE_TEXT_SIZE, Font::HORIZJUST_RIGHT, 
		                                 Font::VERTJUST_TOP, PHONE_TEXT_COLOR);
		toggle_state[i]->changeScale(PHONE_TOGGLE_TEXT_SIZE*1.5f);
	}
}

void CheatCodeMenu::Select(int entry_index)
{
	assert(entry_index < CHEAT_LAST);

	// figure out which cheat that menu entry refers to
	int cheat_index = menu_entry_cheat_index[entry_index];

	// and set that cheat on
	g_session_cheats[cheat_index].turnOn(!g_session_cheats[cheat_index].isOn());
	ResetToggles();
}

void CheatCodeMenu::Select()
{
	Select(highlighted->entry_num);
}

void CheatCodeMenu::OnActivate()
{
	ReOrderEntries(0);
	setHigh(cheats[0]); // Make the first one highlighted
}

void CheatCodeMenu::OnUp(int c)
{
	if (this->highlighted == cheats[0] && next_up == -1)
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	else
		SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);

	if(this->highlighted == cheats[0] && next_up != -1)// if it's the last in the list and there's more below
		ReOrderEntries(next_up);
	else
		FEMultiMenu::OnUp(c);
}

void CheatCodeMenu::OnDown(int c)
{
	if (highlighted == cheats[MAX_CHEATS_PER_SCREEN - 1] && next_down == -1)
		SoundScriptManager::inst()->playEvent(SS_FE_ERROR);
	else
		SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);

	if(highlighted == cheats[MAX_CHEATS_PER_SCREEN - 1] && next_down != -1)// if it's the last in the list and there's more below
		ReOrderEntries(menu_entry_cheat_index[0] + 1);
	else
		FEMultiMenu::OnDown(c);
}

void CheatCodeMenu::SetPQIndices()
{
	arrow_up   = GetPointer("btn_up_on");
	arrow_down = GetPointer("btn_down_on");
}
	
void CheatCodeMenu::ReOrderEntries(const int start)
{
	int i = start, entry_index = 0;

	while(i < CHEAT_LAST && entry_index < MAX_CHEATS_PER_SCREEN)
	{
		if(globalCareerData.isCheatUnlocked(i))
		{
			if(cheats[entry_index] == NULL) // this entry hasn't been created yet
			{
				cheats[entry_index] = NEW FEMenuEntry(ksGlobalTextArray[GT_CHEAT_RAINBOW_SHORT + i], this, false, &frontendmanager.font_body);
				if(entry_index != 0)
				{
					cheats[entry_index]->up = cheats[entry_index - 1];
					cheats[entry_index - 1]->down = cheats[entry_index];
				}
				Add(cheats[entry_index]);
			}
			else // an entry already exists here
			{
				// change its text
				cheats[entry_index]->SetText(ksGlobalTextArray[GT_CHEAT_RAINBOW_SHORT + i]);
			}
			cheats[entry_index]->SetPos(PHONE_SCREEN_LEFT, PHONE_SCREEN_TOP + PHONE_TOGLE_TOP_OFFSET + PHONE_LIST_VERT_SPACING * entry_index);
			cheats[entry_index]->SetVJustify(Font::VERTJUST_TOP);
			cheats[entry_index]->SetHJustify(Font::HORIZJUST_LEFT);
			cheats[entry_index]->SetLineSpacing(frontendmanager.font_body.getGlyph('A')->cell_height*0.6f);
			menu_entry_cheat_index[entry_index] = i;
			entry_index++;
		}
		i++;
	}

	// Now set all the on/off toggles at the right of the screen
	ResetToggles();
	// And make the arrows show or not show as appropriate
	ResetUpDownArrows();
}

void CheatCodeMenu::ResetToggles()
{
	for(int i = 0; i < MAX_CHEATS_PER_SCREEN; i++)
	{
		if(menu_entry_cheat_index[i] != -1)
		{
			if(g_session_cheats[menu_entry_cheat_index[i]].isOn())
				toggle_state[i]->changeText(ksGlobalTextArray[GT_FE_MENU_OP_ON]);
			else
				toggle_state[i]->changeText(ksGlobalTextArray[GT_FE_MENU_OP_OFF]);
		}
		else
			toggle_state[i]->changeText("");
	}
}

void CheatCodeMenu::ResetUpDownArrows()
{
	int i;
	// figure out it there are any more cheats above the current list
	if(menu_entry_cheat_index[0] != 0)
	{
		for(i = menu_entry_cheat_index[0] - 1; i >= 0; i--)
			if(globalCareerData.isCheatUnlocked(i))
				break;
		next_up = i;
	}
	else
		next_up = -1;

	// Ditto below
	if(menu_entry_cheat_index[MAX_CHEATS_PER_SCREEN - 1] != CHEAT_LAST - 1 && menu_entry_cheat_index[MAX_CHEATS_PER_SCREEN - 1] != -1)
	{
		for(i = menu_entry_cheat_index[MAX_CHEATS_PER_SCREEN - 1] + 1; i < CHEAT_LAST; i++)
			if(globalCareerData.isCheatUnlocked(i))
				break;
		if(i != CHEAT_LAST) // if there was another unlocked cheat after the last in our list
			next_down = i;
		else
			next_down = -1;
	}
	else // we're already at the last possible cheat or our list isn't even full yet
		next_down = -1;

	if(next_up != -1)
		arrow_up->TurnOn(true);
	else
		arrow_up->TurnOn(false);

	if(next_down != -1)
		arrow_down->TurnOn(true);
	else
		arrow_down->TurnOn(false);

}

///////////////////////////////////////////////////////////////////////////////////////
// EnterCheatMenu
//
// This is the menu for entering a new cheat code in the phone
///////////////////////////////////////////////////////////////////////////////////////

const int EnterCheatMenu::phone_num_gap[NUM_PHONE_NUM_GAPS] = { 7, 4 }; // the gaps/dashes in the phone number display

EnterCheatMenu::EnterCheatMenu(FEMenuSystem* s, FEManager* man, stringx p, stringx pf_name)
{
	cons(s, man, p, pf_name);
	sys = (GraphicalMenuSystem*) s;
	pq_indices_set = false;

	code_display = NEW MultiLineString(&frontendmanager.font_bold_old, "", PHONE_SCREEN_MIDDLE_X, PHONE_SCREEN_MIDDLE_Y, 
	                                   0, PHONE_TEXT_SIZE*1.18f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, PHONE_TEXT_COLOR);
	code_display->setLineSpacing(PHONE_MAIN_MENU_VERT_SPACING);

	cursor       = NEW TextString(&frontendmanager.font_bold_old, "_", PHONE_SCREEN_MIDDLE_X, PHONE_SCREEN_MIDDLE_Y, 
	                              0, PHONE_TEXT_SIZE*1.18f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, PHONE_TEXT_COLOR);
	cheat_unlocked = -1;
}

EnterCheatMenu::~EnterCheatMenu()	
{
	delete code_display;
	delete cursor;
}

void EnterCheatMenu::Update(time_value_t time_inc)
{
	static float cursor_time = 0.0f;
	static float button_highlight_time = 0.0f;

	cursor_time += time_inc;
	if(cursor_time > PHONE_CURSOR_PERIOD)
		cursor_time = cursor_time - PHONE_CURSOR_PERIOD;
	// Make the cursor fade-blink
	int new_alpha = (int)fabs((cursor_time / PHONE_CURSOR_PERIOD * 2.0f - 1.0f) * 255 * PHONE_CURSOR_INTENSITY);
	if(new_alpha > 255)
		new_alpha = 255;
	cursor->color.set_alpha(new_alpha);

	if(closing)
	{
		closing_timer -= time_inc;
		if(closing_timer <= CHEAT_CODE_MESSAGE_DURATION)
		{
			// if it is a real code, display a success message
			if(cheat_unlocked >= 0)
				code_display->changeText(ksGlobalTextArray[GT_FE_CHEAT_UNLOCKED]);
			else if(current_code.length() == MAX_PHONE_NUM_LEN)
			{
				if(cheat_unlocked == CHEAT_INVALID_CODE)
					code_display->changeText(ksGlobalTextArray[GT_FE_BAD_CHEAT_CODE]);
				else
					code_display->changeText(ksGlobalTextArray[GT_FE_CHEAT_ALREADY_UNLOCKED]);
			}
			else // if exiting the menu without having typed in a full code
				closing_timer = 0.0f; // exit without a messge.
		}
		if(closing_timer <= 0.0f)
			parent->MakeActive(NULL);
	}
	FEMultiMenu::Update(time_inc);
}

void EnterCheatMenu::Draw()
{
	assert(code_display != NULL && cursor != NULL);
	code_display->Draw();
	if(!closing) cursor->Draw();
}

void EnterCheatMenu::Load()
{
	FEMultiMenu::Load();
	SetPQIndices();
}

void EnterCheatMenu::Select(int entry_index)
{
	stringx display_string;
	int len, cur_idx;

	if(closing)
		return;

	if(current_code.length() < MAX_PHONE_NUM_LEN)
		current_code.append((char)('0' + current_button));

	// Now format the current code for displaying.  This will dynamically insert dashes (or whatever)
	// into the phone number as it's being typed, just like a real phone.
	len = current_code.length();
	cur_idx = 0;
	for(int i = 0; i < NUM_PHONE_NUM_GAPS; i++)
	{
		if(len > phone_num_gap[i])
		{
			display_string.append(current_code.slice(cur_idx, len - phone_num_gap[i]));
			display_string.append(PHONE_NUM_GAP_CHAR);
			cur_idx = len - phone_num_gap[i];
		}
	}
	// Paste on the last bit of the number
	display_string.append(current_code.slice(cur_idx, len));

	// Set the display string
	code_display->changeText(display_string);

	// Now update the cursor location
	if(cursor->getText().length() != 0)
	{
		// 1.0f is the scale, both 1.0f's should be replaced if the scale changes
		float text_width = frontendmanager.font_body.getWidth(code_display->getText(), 1.0f, false, 1.0f);
		cursor->changeX(PHONE_SCREEN_MIDDLE_X + text_width * PHONE_TEXT_SIZE * 1.5f * MYSTERY_KLUDGE_VALUE / 2.0f);
		cursor->setHJustify(Font::HORIZJUST_LEFT);
	}

	if(len == MAX_PHONE_NUM_LEN) // if it's a whole phone number then exit the menu & test for cheat code
		ExitMenu(CHEAT_CODE_MESSAGE_DELAY);
}

void EnterCheatMenu::Select()
{
	SoundScriptManager::inst()->playEvent(SS_FE_ONX);
	Select(current_button);
}

void EnterCheatMenu::OnActivate()
{
	current_code = "";
	code_display->changeText("");
	current_button = INITIAL_HIGHLIGHTED_NUMPAD;
	numbers_hi[INITIAL_HIGHLIGHTED_NUMPAD]->TurnOn(true);
	closing = false;
	cheat_unlocked = false;
	cursor->changeX(PHONE_SCREEN_MIDDLE_X);
	cursor->setHJustify(Font::HORIZJUST_CENTER);
}

void EnterCheatMenu::OnUp(int c)
{
	if(closing)
		return;
	SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	if(current_button >= 4 && current_button <= 9)
		ChangeButton(current_button - 3);
	else if(current_button >= 1 && current_button <= 3)
		ChangeButton(0);
	else // current_button == 0
		ChangeButton(8);
}

void EnterCheatMenu::OnDown(int c)
{
	if(closing)
		return;
	SoundScriptManager::inst()->playEvent(SS_FE_UPDOWN);
	if(current_button >= 1 && current_button <= 6)
		ChangeButton(current_button + 3);
	else if(current_button >= 7 && current_button <= 9)
		ChangeButton(0);
	else // current_button == 0
		ChangeButton(2);
}

void EnterCheatMenu::OnRight(int c)
{
	if(closing)
		return;
	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	if(current_button % 3 != 0)           // 1,2,4,5,7,8
		ChangeButton(current_button + 1);
	else if(current_button != 0)          // 3,6,9
		ChangeButton(current_button - 2);
	else                                  // 0
		ChangeButton(9);
}

void EnterCheatMenu::OnLeft(int c)
{
	if(closing)
		return;
	SoundScriptManager::inst()->playEvent(SS_FE_LEFTRIGHT);
	if(current_button == 0)
		ChangeButton(7);
	else if(current_button % 3 != 1)         // 2,3,5,6,8,9
		ChangeButton(current_button - 1);
	else                                     // 1,4,7
		ChangeButton(current_button + 2);
}

void EnterCheatMenu::OnTriangle(int c)
{
	if(!closing)
		ExitMenu(0.0f);
	else
		closing_timer = 0.0f;
	SoundScriptManager::inst()->playEvent(SS_FE_BACK);
}

void EnterCheatMenu::ChangeButton(int new_button)
{
	assert (new_button >= 0 && new_button <= 9);

	numbers_hi[current_button]->TurnOn(false);
	current_button = new_button;
	numbers_hi[current_button]->TurnOn(true);
}

void EnterCheatMenu::SetPQIndices()
{
	stringx number_name;

	if(!pq_indices_set)
	{
		for(int i = 0; i < 10; i++)
		{
			number_name.printf("key_%d_green", i);
			numbers_hi[i] = GetPointer(number_name.c_str());
		}

		pq_indices_set = true;
	}
}

void EnterCheatMenu::ExitMenu(float delay)
{
	closing = true;
	closing_timer = CHEAT_CODE_MESSAGE_DURATION + delay;

	// Check the code against the actual cheat codes
	cheat_unlocked = Cheat::checkCodeUnlock(current_code);
}
