// FEMenu.cpp

#include "global.h"
#include "hwrasterize.h"
#include "FEMenu.h"

#include "FEMenu.h"
#include "app.h"       // bootable builds need this for some reason
#include "game.h"       // bootable builds need this for some reason
#include "FrontEndManager.h"       // bootable builds need this for some reason
#include "camera.h"       // bootable builds need this for some reason
#include "WaveSound.h"


void FEMenuEntry::cons(stringx l, FEMenu* m, bool floating, Font* ft)
{
	menu = m;
	if(!ft) ft = menu->system->font;
	if(floating) text = NEW FloatingText(ft, l);
	else text = NEW MultiLineString(ft, l, 0, 0);
	next = NULL;
	previous = NULL;
	highlight = false;
	disabled = false;
	if(m->flags & FEMENU_HAS_COLOR) text->color = m->color;
	text->no_color = !(menu->flags & FEMENU_HAS_COLOR);
	text->changeScale(m->scale);
	high_ent_color = color32(255, 255, 255, 255);
	norm_ent_color = color32(255, 255, 255, 255);
	ent = NULL;
	up = NULL;
	down = NULL;
	left = NULL;
	right = NULL;
	highlight_intensity = 0.0f;
	has_special_color = false;
	has_special_scale = false;
	highlight_timer = 0.0f;
	no_flash = false;
	disabled_alpha = 64;
}

void FEMenuEntry::Highlight(bool h, bool anim)
{
	highlight = h;
	if(highlight)
	{
		highlight_timer = 0.0f;
		if(ent) ent->set_render_color(high_ent_color);

		if(!has_special_scale && menu->flags & FEMENU_USE_SCALE)
			text->changeScale(menu->scale_high);
		if(has_special_scale) text->changeScale(special_scale_high);

		if(menu->flags & FEMENU_HAS_COLOR_HIGH)
		{
			// kind of a special case
			if(menu->flags & FEMENU_DONT_SKIP_DISABLED && disabled)
			{
				if(has_special_color) text->color = special_color;
				else text->color = menu->color;
				text->color.set_alpha(disabled_alpha);
			}
			else
			{
				if(has_special_color) text->color = special_color_high;
				else text->color = menu->color_high;
			}
			text->no_color = false;
		}
		OnHighlight(anim);
	}
	else
	{
		if(ent) ent->set_render_color(norm_ent_color);
		text->no_color = !disabled && !(menu->flags & FEMENU_HAS_COLOR);
		if(!text->no_color)
		{
			if(has_special_color) text->color = special_color;
			else text->color = menu->color;
			if(disabled) text->color.c.a = disabled_alpha;
		}
		
		if(!has_special_scale && menu->flags & FEMENU_USE_SCALE)
			text->changeScale(menu->scale);
		if(has_special_scale) text->changeScale(special_scale);
	}
}

void FEMenuEntry::Disable(bool d)
{
	disabled = d;
	if(disabled)
	{
		if(has_special_color) text->color = special_color;
		else text->color = menu->color;
		text->color.c.a = disabled_alpha;
		text->no_color = false;		// always has color if disabled, due to alpha
	}
	else
	{
		text->no_color = !highlight && !(menu->flags & FEMENU_HAS_COLOR);
		if(!text->no_color)
			if(highlight)
				if(has_special_color) text->color = special_color_high;
				else text->color = menu->color_high;
			else
				if(has_special_color) text->color = special_color;
				else text->color = menu->color;
	}
}

void FEMenuEntry::Update(time_value_t time_inc)
{
	// Find out how long the pulsation period is from game.ini
	float period = 
		(float)(os_developer_options::inst()->get_int(os_developer_options::INT_MENU_HIGHLIGHT_PERIOD)) / 
		MENU_HIGHLIGHT_MULTIPLIER;

#ifndef M_PI
#define M_PI 3.1415926535897932384626f
#endif
	
	highlight_timer += time_inc;
	highlight_intensity = THROB_INTENSITY * sinf(2*PI*(highlight_timer/period));
//	highlight_intensity /= 8;

//	highlight_intensity += time_inc / period;
	
	// Now make the time be somewhere between -period/2 and +period/2
//	while(highlight_intensity > 1.0f - THROB_INTENSITY)
//		highlight_intensity -= 2.0f * THROB_INTENSITY;

	text->Update(time_inc);
}

void FEMenuEntry::Draw()
{
	uint8 r, g, b, a;
	
	if(highlight && (!(menu->flags & FEMENU_DONT_SKIP_DISABLED) || !disabled))
	{
		color32 ch = menu->color_high;
		color32 c = menu->color;
		if(menu->flags & FEMENU_HAS_COLOR_HIGH_ALT)
			c = menu->color_high_alt;
		if(has_special_color)
		{
			c = special_color;
			ch = special_color_high;
		}

		if(menu->flags & FEMENU_NO_FLASHING || no_flash)
		{
			// this is used to make the beach names not the highlight color
			if(menu->flags & FEMENU_NORM_COLOR_NO_FLASH)
				text->color = c;
			else text->color = ch;
			if(disabled) text->color.set_alpha(disabled_alpha);
		}
		else
		{
			/*
			r = FTOI(ch.get_red()   * (1 - fabs(highlight_intensity)));
			g = FTOI(ch.get_green() * (1 - fabs(highlight_intensity)));
			b = FTOI(ch.get_blue()  * (1 - fabs(highlight_intensity)));
			a = ch.get_alpha();
			*/
			r = FTOI(c.get_red() + (highlight_intensity+.5f)*(ch.get_red() - c.get_red()));
			g = FTOI(c.get_green() + (highlight_intensity+.5f)*(ch.get_green() - c.get_green()));
			b = FTOI(c.get_blue() + (highlight_intensity+.5f)*(ch.get_blue() - c.get_blue()));
			a = ch.get_alpha();
			text->color = color32(r, g, b, a);

			// highlight intensity goes from -.5 to .5
//			if(!has_special_scale) text->changeScale(menu->scale + fabs(highlight_intensity));
//			else text->changeScale(special_scale + fabs(highlight_intensity));
		}
	}
	else
	{
		if(has_special_color) text->color = special_color;
		else text->color = menu->color;
		if(disabled) text->color.set_alpha(disabled_alpha);

//		if(!has_special_scale) text->changeScale(menu->scale);
//		else text->changeScale(special_scale);
	}
	
	text->Draw();
}

void FEMenuEntry::SetSpecialColor(color32 c, color32 ch)
{
	special_color = c;
	special_color_high = ch;
	has_special_color = true;
}

void FEMenuEntry::SetSpecialScale(float s, float sh)
{
	special_scale = s;
	special_scale_high = sh;
	has_special_scale = true;
	text->changeScale(special_scale);
}

bool FEMenuEntry::GetSpecialScale(float &s, float &sh)
{
	if(has_special_scale)
	{
		s = special_scale;
		sh = special_scale_high;
	}
	return has_special_scale;
}

void FEMenuEntry::AddEntity(entity* e, color32 hc, color32 nc)
{
	ent = e;
	high_ent_color = hc;
	norm_ent_color = nc;
	if(highlight) ent->set_render_color(high_ent_color);
	else ent->set_render_color(norm_ent_color);
}

///////////////////////////////////////////////////////////

const int FEMenu::HF_UP			= 0x001;
const int FEMenu::HF_DOWN		= 0x002;
const int FEMenu::HF_LEFT		= 0x004;
const int FEMenu::HF_RIGHT		= 0x008;
const int FEMenu::HF_SELECT		= 0x010;
const int FEMenu::HF_BACK		= 0x020;
const int FEMenu::HF_RESUME		= 0x040;
const int FEMenu::HF_CONTINUE	= 0x080;
const int FEMenu::HF_SWITCH		= 0x100;

FEMenu::FEMenu()
{
	center_x = 0;
	center_y = 0;
	num_entries = 0;
	dy = 0;
	half = 0;
	menu_num = 0;
	init = false;
	scale = 1.0f;
	scale_high = 1.0f;
	max_vis_entries = 1;
	flags = 0;

	entries = NULL;
	first_vis_entry = NULL;
	last_vis_entry = NULL;
	highlighted = NULL;
	system = NULL;
	back = NULL;
	back_num = 0;

	submenus = NULL;
	active = NULL;
	parent = NULL;
	helpText = NULL;
	helpFlags = 0;

	next_sub = 0;

}

void FEMenu::cons(FEMenuSystem* s, int x, int y, color32 c, color32 ch, color32 cha, float sc, float sch, int mve, int flg)
{
	entries = NULL;
	first_vis_entry = NULL;
	last_vis_entry = NULL;
	highlighted = NULL;
	back = NULL;
	system = s;
	flags = flg;

	helpText = NEW TextString(&system->manager->font_info, "", 320, 365, 800, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, system->manager->col_info_g);
	helpText->setButtonScale(1.0f);
	SetHelpText(HF_UP | HF_DOWN | HF_SELECT | HF_RESUME);
	
	// scrolling will be set true if more than max_vis_entries
	//	scrolling = false;
	//	wrap = false;
	center_x = x;
	center_y = y;
	num_entries = 0;
	dy = 28;
	half = 0;
	init = false;
	//	has_color = hc;
	//	has_color_high = hch;
	
	if (c.get_alpha() == 0) flags = (flags & !FEMENU_HAS_COLOR);
	else
	{
		flags |= FEMENU_HAS_COLOR;
		color = c;
	}
	if (ch.get_alpha() == 0) flags = (flags & !FEMENU_HAS_COLOR_HIGH);
	else
	{
		flags |= FEMENU_HAS_COLOR_HIGH;
		color_high = ch;
	}
	if (cha.get_alpha() == 0) flags = (flags & !FEMENU_HAS_COLOR_HIGH_ALT);
	else
	{
		flags |= FEMENU_HAS_COLOR_HIGH_ALT;
		color_high_alt = cha;
	}

	flags |= FEMENU_USE_SCALE;
	scale = sc;
	scale_high = sch;
	active = NULL;
	next_sub = NULL;
	parent = NULL;
	max_vis_entries = mve;
}

FEMenu::~FEMenu()
{
	FEMenuEntry* tmp = entries;
	FEMenuEntry* tmp2;
	while(tmp)
	{
		tmp2 = tmp->next;
		delete tmp;
		tmp = tmp2;
	}

	delete helpText;
}

void FEMenu::setHigh(FEMenuEntry* e, bool anim)
{
	assert(e);
	if(highlighted) highlighted->Highlight(false);
	e->Highlight(true, anim);
	highlighted = e;
}

void FEMenu::Add(FEMenuEntry* e)
{
	assert(e);
	
	// creating a doubly-linked list in order
	// the last menu in the list will have next = NULL, but entries->previous
	// *will* point to the last element
	if(entries)
	{
		if(entries->previous)
		{
			entries->previous->next = e;
			e->previous = entries->previous;
		}
		else
		{
			entries->next = e;
			e->previous = entries;
		}
		entries->previous = e;
	}
	else
	{
		entries = e;
		e->previous = NULL;
	}
	e->next = NULL;
	e->entry_num = num_entries;
	num_entries++;
}

void FEMenu::Next()
{
	assert(highlighted);
	if(!(flags & FEMENU_WRAP))
	{
		if(highlighted == entries->previous) return;
		FEMenuEntry* tmp = highlighted->next;

		// loop through until end of list or hit a not-disabled entry.
		// if we hit the bottom of the list first, then there's nowhere else to go
		while(tmp && tmp->GetDisable()) tmp = tmp->next;
		if(!tmp) return;
	}
	if((flags & FEMENU_SCROLLING) && highlighted == last_vis_entry)
	{
		FEMenuEntry* first = first_vis_entry->next;
		if(!first) first = entries;
		if(flags & FEMENU_DONT_SHOW_DISABLED)
		{
			while(first->GetDisable() && first != highlighted)
			{
				first = first->next;
				if(!first) first = entries;
			}
		}
		setVis(first);
	}
	
	FEMenuEntry* tmp = highlighted->next;
	if(!tmp) tmp = entries;
	if(!(flags & FEMENU_DONT_SKIP_DISABLED))
	{
		while(tmp->GetDisable() && tmp != highlighted)
		{
			tmp = tmp->next;
			if(!tmp) tmp = entries;
		}
	}
	setHigh(tmp);
}

void FEMenu::Previous()
{
	assert(highlighted);
	if(!(flags & FEMENU_WRAP) && highlighted == entries) return;
	if((flags & FEMENU_SCROLLING) && highlighted == first_vis_entry)
	{
		FEMenuEntry* first = first_vis_entry->previous;
		if(flags & FEMENU_DONT_SHOW_DISABLED)
		{
			while(first->GetDisable() && first != highlighted)
				first = first->previous;
		}
		setVis(first);
	}
	
	FEMenuEntry* tmp = highlighted->previous;
	if(tmp)
	{
		if(!(flags & FEMENU_DONT_SKIP_DISABLED))
			while(tmp->GetDisable() && tmp != highlighted) tmp = tmp->previous;
			setHigh(tmp);
	}
}

void FEMenu::Select()
{
	assert(highlighted);
	system->Select(menu_num, highlighted->entry_num);
}

void FEMenu::setVis(FEMenuEntry* first)
{
	first_vis_entry = first;
	FEMenuEntry* tmp = first;
	int y;
	for(int i=0; i<max_vis_entries; i++)
	{
		if(tmp == NULL) tmp = entries;
		while(flags & FEMENU_DONT_SHOW_DISABLED && tmp->GetDisable())
		{
			tmp = tmp->next;
			if(tmp == NULL) tmp = entries;
		}
		y = center_y + i * dy - half;
		tmp->SetPos(center_x, y);
		if(i == max_vis_entries-1) last_vis_entry = tmp;
		tmp = tmp->next;
	}
}

void FEMenu::Init()
{
	if(num_entries > max_vis_entries) flags |= FEMENU_SCROLLING;
	if(flags & FEMENU_SCROLLING) half = (dy * (max_vis_entries-1))/2;
	else if(flags & FEMENU_DONT_SHOW_DISABLED)
	{
		// count up non-disabled entries
		int count = 0;
		FEMenuEntry* tmp = entries;
		while(tmp != NULL)
		{
			if(!tmp->GetDisable()) count++;
			tmp = tmp->next;
		}
		half = (dy * (count-1))/2;
	}
	else half = (dy * (num_entries-1))/2;


	if (entries)
		setHigh(entries, false);
	init = true;
	
	if(flags & FEMENU_SCROLLING) setVis(entries);
	else
	{
		int y;
		FEMenuEntry* tmp = entries;
		int index = 0;
		while(tmp != NULL)
		{
			if(!(flags & FEMENU_DONT_SHOW_DISABLED) || !tmp->GetDisable())
			{
				y = center_y + index * dy - half;
				tmp->SetPos(center_x, y);
				index++;
			}
			tmp = tmp->next;
		}
	}
}

void FEMenu::Draw()
{
	if (active != NULL) active->Draw();
	else
	{
		if(!(flags & FEMENU_SCROLLING))
		{
			FEMenuEntry* tmp = entries;
			while(tmp != NULL)
			{

				if (!(flags & FEMENU_DONT_SHOW_DISABLED && tmp->GetDisable()))
					tmp->Draw();
				tmp = tmp->next;
			}
		}
		else
		{
			FEMenuEntry* tmp = first_vis_entry;
			int count = max_vis_entries;
			while(count > 0)
			{
				count--;
				if(!tmp) tmp = entries;
				while(flags & FEMENU_DONT_SHOW_DISABLED && tmp->GetDisable())
				{
					tmp = tmp->next;
					if(tmp == NULL) tmp = entries;
				}
				tmp->Draw();
				tmp = tmp->next;
			}
		}

		// Draw help text.
		helpText->Draw();
	}
}

void FEMenu::Update(time_value_t time_inc)
{
	FEMenuEntry* tmp = entries;
	while(tmp)
	{
		tmp->Update(time_inc);
		tmp = tmp->next;
	}
}

void FEMenu::SetHelpText(const int hFlags)
{
	stringx		helpStr;
	stringx		spacingStr(" ");
	bool		chooseText = false;
	int			numSections = 0;		// number of commas required
	int			i = 0;
	
	helpFlags = hFlags;

	// Count the number of commas required.
	if ((helpFlags & HF_UP) || (helpFlags & HF_DOWN) || (helpFlags & HF_LEFT) || (helpFlags & HF_RIGHT))
	{
		numSections++;
		chooseText = true;
	}
	if (helpFlags & HF_SELECT) numSections++;
	if (helpFlags & HF_BACK) numSections++;
//#if !defined(TARGET_XBOX)
	if (helpFlags & HF_RESUME) numSections++;
//#endif
	if (helpFlags & HF_CONTINUE) numSections++;
	if (helpFlags & HF_SWITCH) numSections++;

	// Add text for resuming.
//#if !defined(TARGET_XBOX)
	if (helpFlags & HF_RESUME)
	{
		helpStr += ksGlobalButtonArray[GT_PadStart] + ksGlobalTextArray[GT_MENU_CONTINUE];
		if (++i < numSections)
			helpStr += spacingStr;
	}
//#endif
		
	// Add text for movement.
	if (helpFlags & HF_UP)
		helpStr += ksGlobalButtonArray[GT_PadU];
	if (helpFlags & HF_DOWN)
		helpStr += ksGlobalButtonArray[GT_PadD];
	if (helpFlags & HF_LEFT)
		helpStr += ksGlobalButtonArray[GT_PadL];
	if (helpFlags & HF_RIGHT)
		helpStr += ksGlobalButtonArray[GT_PadR];
	if (chooseText)
	{
		helpStr += stringx(" ") + ksGlobalTextArray[GT_MENU_MOVE];
		if (++i < numSections)
			helpStr += spacingStr;
	}

	// Add text for switching.
	if (helpFlags & HF_SWITCH)
	{
		helpStr += ksGlobalButtonArray[GT_PadL] + ksGlobalButtonArray[GT_PadR] + stringx(" ") + ksGlobalTextArray[GT_FE_MENU_SWITCH];
		if (++i < numSections)
			helpStr += spacingStr;
	}

	// Add text for selecting.
	if (helpFlags & HF_SELECT)
	{
		helpStr += ksGlobalButtonArray[GT_PadCross] + stringx(" ") + ksGlobalTextArray[GT_MENU_SELECT];
		if (++i < numSections)
			helpStr += spacingStr;
	}

	// Add text for going back.
	if (helpFlags & HF_BACK)
	{
		helpStr += ksGlobalButtonArray[GT_PadBack] + stringx(" ") + ksGlobalTextArray[GT_MENU_BACK];
		if (++i < numSections)
			helpStr += spacingStr;
	}

	// Add text continuing.
	if (helpFlags & HF_CONTINUE)
	{
		helpStr += ksGlobalButtonArray[GT_PadCross] + ksGlobalTextArray[GT_MENU_CONTINUE];
		if (++i < numSections)
			helpStr += spacingStr;
	}
	
	helpText->changeText(helpStr);
}

void FEMenu::AddSubmenu(FEMenu* sub)
{
	sub->next_sub = submenus;
	submenus = sub;
	sub->parent = this;
}

void FEMenu::MakeActive(FEMenu* a)
{
	if(active) active->OnUnactivate(a);
	else OnUnactivate(a);
	if(a) a->OnActivate();
	else OnActivate();
	active = a;
}

// Sets the highlight to the first non-disabled entry.
void FEMenu::HighlightDefault(void)
{
	FEMenuEntry* tmp = entries;

	while (tmp && tmp->GetDisable())
		tmp = tmp->next;

	if (tmp)
		setHigh(tmp, false);
}

// if FEMENU_DONT_SHOW_DISABLED, disable as necessary before calling this
// default ::OnActivate function
void FEMenu::OnActivate()
{
	active = NULL;

	HighlightDefault();

	if (flags & FEMENU_SCROLLING) setVis(highlighted);
	else if (flags & FEMENU_DONT_SHOW_DISABLED) Init();
}

void FEMenu::OnSelect(int c)
{
#ifdef TARGET_XBOX
	// If we're on the xbox then the Select button is actually the Back button, and it's supposed to be
	// mapped to the same thing as triangle on PS2.  So call OnTriangle()
	OnTriangle(c);
#endif
}

void FEMenu::OnStart(int c)
{
	if (active)
		active->OnStart(c);
	else
	{
#ifdef TARGET_XBOX

		bool cross = true;

		// If we're on the xbox then the Start button is supposed to do the same thing as 
		// cross on PS2, so call OnCross().
		// Except for certain pause menus, where we want Start to toggle the menu on/off.
		if (system == frontendmanager.pms &&
			frontendmanager.pms->IsResumable(frontendmanager.pms->menus[frontendmanager.pms->active]))
			cross = false;
		
		if (cross)
			OnCross(c);
#endif
	}
}

void FEMenu::OnTriangle(int c)
{
		if(parent) parent->MakeActive(NULL);
		else if(back) system->MakeActive(back->menu_num, back_num);
}

void FEMenu::OnCross(int c)
{
	if(active) active->OnCross(c);
	else if(highlighted != NULL && !highlighted->GetDisable()) Select();
}

void FEMenu::SetAllScale(float s)
{
	float s1, sh1;
	FEMenuEntry* tmp = entries;
	while(tmp != NULL)
	{
		if(tmp->GetSpecialScale(s1, sh1))
			tmp->SetSpecialScale(s * s1, s * sh1);
		tmp = tmp->next;
	}
	scale = scale * s;
	scale_high = scale_high * s;
}

// BETH added this just to make the code a bit cleaner
void FEMenu::play_sound(const char* name)
{
	if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		nslSourceId s = nslLoadSource(name);
		if (s != NSL_INVALID_ID)
		{
			nslSoundId snd = nslAddSound(s);
			if (snd != NSL_INVALID_ID)
				nslPlaySound(snd);
		}
		else debug_print("FIXME: MISSING %s SOUND\n", name);
	}
}

/**************************** FrontEnd ******************************/

void FrontEnd::cons(FEManager* man, stringx p, stringx pf_name)
{
	panel = PanelFile(pf_name, p);
	manager = man;
	path = p;
}

void FrontEnd::Draw()
{
	panel.Draw(0);
}

void FrontEnd::Update(time_value_t time_inc)
{
	panel.Update(time_inc);
	pam.UpdateAnims(time_inc);
}

/********************* FEGraphicalMenuEntry ***************************/

FEGraphicalMenuEntry::FEGraphicalMenuEntry(FEMenu* m)
{
	FEMenuEntry::cons("", m);
	pq = NULL;
	pq_high = NULL;
	highlight_paf = NULL;
	pam = NULL;
	already_playing = false;
}

FEGraphicalMenuEntry::FEGraphicalMenuEntry(FEMenu* m, PanelQuad* quad_normal, PanelQuad* quad_high)
{
	FEMenuEntry::cons("", m);
	pq = quad_normal;
	pq_high = quad_high;
	pq->AddedToMenu();
	if(pq_high) pq_high->AddedToMenu();
	highlight_paf = NULL;
	pam = NULL;
	already_playing = false;
}

FEGraphicalMenuEntry::FEGraphicalMenuEntry(FEMenu* m, PanelQuad* quad_normal, PanelAnimFile* pf, PanelAnimManager* pm, PanelQuad* quad_high)
{
	FEMenuEntry::cons("", m);
	pq = quad_normal;
	pq->AddedToMenu();
	pq_high = quad_high;
	if(pq_high) pq_high->AddedToMenu();
	highlight_paf = pf;
	pam = pm;
	already_playing = false;
}

void FEGraphicalMenuEntry::Load(PanelQuad* quad_normal, PanelQuad* quad_high)
{
	pq = quad_normal;
	pq_high = quad_high;
	pq->AddedToMenu();
	if(pq_high) pq_high->AddedToMenu();
}

void FEGraphicalMenuEntry::Load(PanelQuad* quad_normal, PanelAnimFile* pf, PanelAnimManager* pm, PanelQuad*
                                quad_high)
{
	pq = quad_normal;
	pq->AddedToMenu();
	pq_high = quad_high;
	if(pq_high) pq_high->AddedToMenu();
	highlight_paf = pf;
	pam = pm;
}

void FEGraphicalMenuEntry::Draw()
{
	// for now, just make the layer -1 to signify that's its a menu
	if(highlight && pq_high)
		pq_high->Draw(-1);
	else
	{
		if(!pq) return;
		if(disabled)
			pq->Draw(-1, .5f);
		else
			pq->Draw(-1);
	}
}

void FEGraphicalMenuEntry::OnHighlight(bool anim)
{
	if(highlight_paf && anim)
	{
		assert(pam);
		if(already_playing)
			pam->Play(highlight_paf, PLAY, HOLD_AFTER_PLAYING | ALREADY_PLAYING);
		else
		{
			pam->Play(highlight_paf, PLAY, HOLD_AFTER_PLAYING);
			already_playing = true;
		}
	}
}

void FEGraphicalMenuEntry::TurnOn(bool on)
{
	if (pq)	pq->TurnOn(on);
	if(pq_high) pq_high->TurnOn(on);
}


/********************* FEGraphicalMenu ********************************/

void FEGraphicalMenu::cons(FEMenuSystem* s, FEManager* man, stringx path, stringx pf_name)
{
	entries = NULL;
	highlighted = NULL;
	back = NULL;
	system = s;
	num_entries = 0;
	init = false;
	active = NULL;
	next_sub = NULL;
	parent = NULL;

	// Ick.
	helpText = NEW TextString(&man->font_info, "", 320, 424, 20, 0.8f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, man->col_info_g);
	helpText->setButtonScale(1.0f);
	SetHelpText(HF_CONTINUE);
	
	// default values for FEMenu stuff
	center_x = 100;
	center_y = 100;
	flags = 0;
	flags |= FEMENU_HAS_COLOR;
	flags |= FEMENU_HAS_COLOR_HIGH;
	flags |= FEMENU_HAS_COLOR_HIGH_ALT;
	color = man->col_unselected;
	color_high = man->col_highlight;
	color_high_alt = man->col_highlight2;
	max_vis_entries = 1;
	scale = 1;
	scale_high = 1;
	FrontEnd::cons(man, path, pf_name);
}

void FEGraphicalMenu::Next()
{
	assert(highlighted);
	FEMenuEntry* tmp = highlighted->next;
	if(!tmp) tmp = entries;
	if(!(flags & FEMENU_DONT_SKIP_DISABLED))
		while(tmp->GetDisable() && tmp != highlighted)
		{
			tmp = tmp->next;
			if(!tmp) tmp = entries;
		}
		setHigh(tmp);
}

void FEGraphicalMenu::Previous()
{
	assert(highlighted);
	FEMenuEntry* tmp = highlighted->previous;
	if(tmp)
	{
		if(!(flags & FEMENU_DONT_SKIP_DISABLED))
			while(tmp->GetDisable() && tmp != highlighted)
				tmp = tmp->previous;
			setHigh(tmp);
	}
}

void FEGraphicalMenu::Init()
{
	if(entries)
		setHigh(entries);
	init = true;
}

void FEGraphicalMenu::Draw()
{
	// if no parent, then we must be top level
	if(!parent) panel.Draw(0);
	if(active != NULL)
		active->Draw();
	else
	{
		FEMenuEntry* tmp = entries;
		while(tmp != NULL)
		{
			if (!(flags & FEMENU_DONT_SHOW_DISABLED && tmp->GetDisable()))
				tmp->Draw();
			tmp = tmp->next;
		}
	}
	if(!parent) panel.Draw(1);
}

void FEGraphicalMenu::Select(int entry_num)
{
	if(active != NULL) active->Select(entry_num);
	else FEMenu::Select(entry_num);
}


void FEGraphicalMenu::OnActivate()
{
	active = NULL;
	//	if(entries) setHigh(entries, false);
	FEMenuEntry* tmp = entries;
	if(!(flags & FEMENU_DONT_SKIP_DISABLED))
		while(tmp)
		{
			if(!tmp->GetDisable()) break;
			tmp = tmp->next;
		}
		if(tmp) setHigh(tmp, false);
}

/********************* FETextMultiMenu ************************************/

void FETextMultiMenu::cons(FEMenuSystem* s, color32 c, color32 ch, float sc, float sch, int flg)
{
	entries = NULL;
	first_vis_entry = NULL;
	last_vis_entry = NULL;
	highlighted = NULL;
	back = NULL;
	system = s;
	// will be set true if more than max_vis_entries
	//	scrolling = false;
	//	wrap = false;
	flags = flg;
	num_entries = 0;
	dy = 28;
	half = 0;
	init = false;
	//	has_color = hc;
	//	has_color_high = hch;
	if(c.get_alpha() == 0) flags = !(flags & FEMENU_HAS_COLOR);
	else
	{
		flags |= FEMENU_HAS_COLOR;
		color = c;
	}
	if(ch.get_alpha() == 0) flags = !(flags & FEMENU_HAS_COLOR_HIGH);
	else
	{
		flags |= FEMENU_HAS_COLOR_HIGH;
		color_high = ch;
	}
	scale = sc;
	scale_high = sch;
}

void FETextMultiMenu::Up()
{
	assert(highlighted);
	FEMenuEntry* tmp = highlighted->up;
	if(!(flags & FEMENU_DONT_SKIP_DISABLED))
	{
		while(tmp && tmp->GetDisable() && tmp != highlighted)
			tmp = tmp->up;
		if(tmp == highlighted) return;
	}
	if(tmp) setHigh(tmp);
}

void FETextMultiMenu::Down()
{
	assert(highlighted);
	FEMenuEntry* tmp = highlighted->down;
	if(!(flags & FEMENU_DONT_SKIP_DISABLED))
	{
		while(tmp && tmp->GetDisable() && tmp != highlighted)
			tmp = tmp->down;
		if(tmp == highlighted) return;
	}
	if(tmp) setHigh(tmp);
}

void FETextMultiMenu::Left()
{
	assert(highlighted);
	FEMenuEntry* tmp = highlighted->left;
	if(!(flags & FEMENU_DONT_SKIP_DISABLED))
	{
		while(tmp && tmp->GetDisable() && tmp != highlighted)
			tmp = tmp->left;
		if(tmp == highlighted) return;
	}
	if(tmp) 
	{
		
		setHigh(tmp);
	}
	
}

void FETextMultiMenu::Right()
{
	assert(highlighted);
	FEMenuEntry* tmp = highlighted->right;
	if(!(flags & FEMENU_DONT_SKIP_DISABLED))
	{
		while(tmp && tmp->GetDisable() && tmp != highlighted)
			tmp = tmp->right;
		if(tmp == highlighted) return;
	}
	if(tmp) 
	{
	
		setHigh(tmp);
	}
	
}


/********************* FEMultiMenu ************************************/

void FEMultiMenu::Up()
{
	assert(highlighted);
	FEMenuEntry* tmp = highlighted->up;
	if(!(flags & FEMENU_DONT_SKIP_DISABLED))
	{
		while(tmp && tmp->GetDisable() && tmp != highlighted)
			tmp = tmp->up;
		if(tmp == highlighted) return;
	}
	if(tmp) setHigh(tmp);
}

void FEMultiMenu::Down()
{
	assert(highlighted);
	FEMenuEntry* tmp = highlighted->down;
	if(!(flags & FEMENU_DONT_SKIP_DISABLED))
	{
		while(tmp && tmp->GetDisable() && tmp != highlighted)
			tmp = tmp->down;
		if(tmp == highlighted) return;
	}
	if(tmp) setHigh(tmp);
}

void FEMultiMenu::Left()
{
	assert(highlighted);
	FEMenuEntry* tmp = highlighted->left;
	if(!(flags & FEMENU_DONT_SKIP_DISABLED))
	{
		while(tmp && tmp->GetDisable() && tmp != highlighted)
			tmp = tmp->left;
		if(tmp == highlighted) return;
	}
	if(tmp) 
	{
		
		setHigh(tmp);
	}
	
}

void FEMultiMenu::Right()
{
	assert(highlighted);
	FEMenuEntry* tmp = highlighted->right;
	if(!(flags & FEMENU_DONT_SKIP_DISABLED))
	{
		while(tmp && tmp->GetDisable() && tmp != highlighted)
			tmp = tmp->right;
		if(tmp == highlighted) return;
	}
	if(tmp) 
	{
		
		setHigh(tmp);
	}
	
}

void FEMultiMenu::SetSecondaryCursor(FEMenuEntry* e, bool anim)
{
	if(secondary_cursor) secondary_cursor->Highlight(false);
	if(e) e->Highlight(true, anim);
	secondary_cursor = e;
}

/********************* FEMenuSystem ***********************************/

void FEMenuSystem::cons(int s, FEManager* man, Font* f)
{
	size = s;
#if defined(TARGET_XBOX) || defined(TARGET_GC)
	menus = new FEMenu *[size];
#else
	menus = NEW (FEMenu*)[size];
#endif /* TARGET_XBOX JIV DEBUG */
	count = 0;
	manager = man;
	font = f;
	active = -1;
	device_flags = 0xffffffff;
}

void FEMenuSystem::Add(FEMenu* m)
{
	assert(count < size);
	menus[count] = m;
	if(menus[count])
		menus[count]->menu_num = count;
	count++;
}

void FEMenuSystem::InitAll()
{
	for(int i=0; i<FEMENUCMD_END; i++)
		for(int j=0; j<NUM_CONTROLLER_PORTS; j++)
			button_down[i][j] = getButtonState(i, j);
		for(int i=0; i<count; i++)
			menus[i]->Init();
}

void FEMenuSystem::MakeActive(int index, int sub_menu)
{
	assert(index < size);
	
	if (active != -1)
		menus[active]->OnUnactivate(menus[index]);

	if (sub_menu == 1)
		menus[index]->OnActivate();
	else
		menus[index]->OnActivate(sub_menu);

	active = index;
}

void FEMenuSystem::Update(time_value_t time_inc)
{
	int i, j;
	
	assert(menus[active]);
	menus[active]->Update(time_inc);
	
	// checks for all input
	for(i=0; i<FEMENUCMD_END; i++)
		for(j=0; j<NUM_CONTROLLER_PORTS; j++)
		{
			if(button_down[i][j] && !getButtonState(i, j))
			{
				button_down[i][j] = false;
				if( device_flags & ( 1 << j ) )
					menus[active]->OnButtonRelease(j, i);
				return;	// bad things happen if we register two button actions on the same frame (dc 07/08/02)
			}
			else if(!button_down[i][j] && getButtonState(i, j))
			{
				button_down[i][j] = true;
				if( device_flags & ( 1 << j ) )
					OnButtonPress(i, j);
				return;	// bad things happen if we register two button actions on the same frame (dc 07/08/02)
			}
		}
}

void FEMenuSystem::UpdateButtonDown()
{
	// Update button_down array
	for(int i=0; i<FEMENUCMD_END; i++)
		for(int j=0; j<NUM_CONTROLLER_PORTS; j++)
			button_down[i][j] = getButtonState(i, j);
}

void FEMenuSystem::OnButtonPress(int button, int controller)
{
	menus[active]->OnAnyButtonPress(controller, button);
	switch(button)
	{
	case FEMENUCMD_SELECT  : menus[active]->OnSelect(controller); break;
	case FEMENUCMD_START   : menus[active]->OnStart(controller); break;
	case FEMENUCMD_UP      : menus[active]->OnUp(controller); break;
	case FEMENUCMD_DOWN    : menus[active]->OnDown(controller); break;
	case FEMENUCMD_LEFT    : menus[active]->OnLeft(controller); break;
	case FEMENUCMD_RIGHT   : menus[active]->OnRight(controller); break;
	case FEMENUCMD_CROSS   : menus[active]->OnCross(controller); break;
	case FEMENUCMD_TRIANGLE: menus[active]->OnTriangle(controller); break;
	case FEMENUCMD_SQUARE  : menus[active]->OnSquare(controller); break;
	case FEMENUCMD_CIRCLE  : menus[active]->OnCircle(controller); break;
	case FEMENUCMD_L1      : menus[active]->OnL1(controller); break;
	case FEMENUCMD_R1      : menus[active]->OnR1(controller); break;
	case FEMENUCMD_L2      : menus[active]->OnL2(controller); break;
	case FEMENUCMD_R2      : menus[active]->OnR2(controller); break;
	default: assert(0);
	}
}

/////////////////////////////////////////////////////////////

// Tests if "button" pressed on *any* controller
int getButtonPressed(int button)
{
	input_mgr* im = input_mgr::inst();
	if(im->get_control_state(ANY_LOCAL_JOYSTICK, button) == AXIS_MAX)
		return 1;
	return 0;
}

// Tests if "button" pressed only on "controller" controller
int getButtonPressed(int button, int controller)
{
	input_mgr* im = input_mgr::inst();
	device_id_t cont;
	
	assert(controller < NUM_CONTROLLER_PORTS);
	switch(controller)
	{
	case 0: cont = JOYSTICK1_DEVICE; break;
	case 1: cont = JOYSTICK2_DEVICE; break;
	case 2: cont = JOYSTICK3_DEVICE; break;
	default: cont = JOYSTICK4_DEVICE; break;
	}
	
	if(im->get_control_state(cont, button) == AXIS_MAX)
		return 1;
	return 0;
}

// Tests if "button" pressed on *any* controller
int getAnalogState(int button)
{
	input_mgr* im = input_mgr::inst();
	rational_t state = im->get_control_state(ANY_LOCAL_JOYSTICK, button);
	if(state > AXIS_MID)
		return 1;
	else if (state < AXIS_MID)
		return -1;
	return 0;
}

// Tests if "button" pressed only on "controller" controller
int getAnalogState(int button, int controller)
{
	input_mgr* im = input_mgr::inst();
	device_id_t cont;
	
	assert(controller < NUM_CONTROLLER_PORTS);
	switch(controller)
	{
	case 0: cont = JOYSTICK1_DEVICE; break;
	case 1: cont = JOYSTICK2_DEVICE; break;
	case 2: cont = JOYSTICK3_DEVICE; break;
	default: cont = JOYSTICK4_DEVICE; break;
	}
	
	rational_t state = im->get_control_state(cont, button);
	if(state > AXIS_MID)
		return 1;
	else if (state < AXIS_MID)
		return -1;
	return 0;
}

// Here's a kludge to make it so that the "B" button backs out of menus on the xbox.
// Jesus cries when we make kludges....
// ....and maps the B button on the gamecube, sweet lord!
#if defined(TARGET_XBOX)
#define BACK_OUT_BUTTON     PSX_CIRCLE
#define NON_BACK_OUT_BUTTON_1 PSX_TRIANGLE
#define NON_BACK_OUT_BUTTON_2 PSX_SQUARE
#elif defined(TARGET_GC)
#define BACK_OUT_BUTTON     PSX_SQUARE
#define NON_BACK_OUT_BUTTON_1 PSX_TRIANGLE
#define NON_BACK_OUT_BUTTON_2 PSX_CIRCLE
#else
#define BACK_OUT_BUTTON     PSX_TRIANGLE
#define NON_BACK_OUT_BUTTON_1 PSX_CIRCLE
#define NON_BACK_OUT_BUTTON_2 PSX_SQUARE
#endif

// Tests if "button" pressed on *any* controller
int getButtonState(int button)
{
	switch(button)
	{
	case FEMENUCMD_SELECT  : return getButtonPressed(PSX_SELECT);
	case FEMENUCMD_START   : return getButtonPressed(PSX_START);
	case FEMENUCMD_UP      : return (getAnalogState(PSX_UD) == -1)?1:0;
	case FEMENUCMD_DOWN    : return (getAnalogState(PSX_UD) == 1)?1:0;
	case FEMENUCMD_LEFT    : return (getAnalogState(PSX_LR) == -1)?1:0;
	case FEMENUCMD_RIGHT   : return (getAnalogState(PSX_LR) == 1)?1:0;
	case FEMENUCMD_CROSS   : return getButtonPressed(PSX_X);
	case FEMENUCMD_TRIANGLE: return getButtonPressed(BACK_OUT_BUTTON);
	case FEMENUCMD_CIRCLE  : return getButtonPressed(NON_BACK_OUT_BUTTON_1);
	case FEMENUCMD_SQUARE  : return getButtonPressed(NON_BACK_OUT_BUTTON_2);
	case FEMENUCMD_L1      : return getButtonPressed(PSX_L1);
	case FEMENUCMD_R1      : return getButtonPressed(PSX_R1);
	case FEMENUCMD_L2      : return getButtonPressed(PSX_L2);
	case FEMENUCMD_R2      : return getButtonPressed(PSX_R2);
	default: return 0;
	}
}

// Tests if "button" pressed only on "controller" controller
int getButtonState(int button, int controller)
{
	switch(button)
	{
	case FEMENUCMD_SELECT  : return getButtonPressed(PSX_SELECT, controller);
	case FEMENUCMD_START   : return getButtonPressed(PSX_START, controller);
	case FEMENUCMD_UP      : return (getAnalogState(PSX_UD, controller) == -1)?1:0;
	case FEMENUCMD_DOWN    : return (getAnalogState(PSX_UD, controller) == 1)?1:0;
	case FEMENUCMD_LEFT    : return (getAnalogState(PSX_LR, controller) == -1)?1:0;
	case FEMENUCMD_RIGHT   : return (getAnalogState(PSX_LR, controller) == 1)?1:0;
	case FEMENUCMD_CROSS   : return getButtonPressed(PSX_X, controller);
	case FEMENUCMD_TRIANGLE: return getButtonPressed(BACK_OUT_BUTTON, controller);
	case FEMENUCMD_CIRCLE  : return getButtonPressed(NON_BACK_OUT_BUTTON_1, controller);
	case FEMENUCMD_SQUARE  : return getButtonPressed(NON_BACK_OUT_BUTTON_2, controller);
	case FEMENUCMD_L1      : return getButtonPressed(PSX_L1, controller);
	case FEMENUCMD_R1      : return getButtonPressed(PSX_R1, controller);
	case FEMENUCMD_L2      : return getButtonPressed(PSX_L2, controller);
	case FEMENUCMD_R2      : return getButtonPressed(PSX_R2, controller);
	default: return 0;
	}
}
