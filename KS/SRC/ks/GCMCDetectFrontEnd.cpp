#include "global.h"

#include "GCMCDetectFrontEnd.h"
#include "FEPAnel.h"
#include <time.h>
#include "FEMenu.h"
#include "FrontEndManager.h"
#include "osGameSaver.h"

GCMCDetectFrontEnd::GCMCDetectFrontEnd(FEMenuSystem* s, FEManager* man, stringx p, stringx pq)
{
	cons(s, man, p, pq);
	flags |= FEMENU_DONT_SHOW_DISABLED | FEMENU_HAS_COLOR_HIGH_ALT;

	color = manager->col_unselected;
	color_high = manager->col_highlight;
	color_high_alt = manager->col_highlight2;
	scale = scale_high = 1.2f;
	
	foundGame = false;

	entries[MCContinue] = NEW FEMenuEntry("", this, false, &man->font_body);
	Add(entries[MCContinue] );
	entries[MCOther] = NEW FEMenuEntry("", this, false, &man->font_body);
	Add(entries[MCOther] );
	entries[MCRetry] = NEW FEMenuEntry("", this, false, &man->font_body);
	Add(entries[MCRetry]);
	
	mc_state = MC_NONE;
}

GCMCDetectFrontEnd::~GCMCDetectFrontEnd()
{
	delete error;
}

void GCMCDetectFrontEnd::Init()
{
	
	leftx = 75;
	uppery = 100;
	width = 640 - leftx*2 - 90;	// 90 for extra spacing -beth
	height = 300;
	entries[MCContinue]->SetPos(leftx + 140, 300);
	entries[MCContinue]->right = entries[MCOther];
	entries[MCOther]->left = entries[MCContinue];
	entries[MCOther]->right = entries[MCRetry];
	entries[MCOther]->SetPos( 320, 300 );
	entries[MCRetry]->left = entries[MCOther];
	entries[MCRetry]->SetPos(640 - leftx - 140, 300);
	
	error = NEW BoxText(&frontendmanager.font_body, "", 320,uppery+80);
	error->changeScale(.75f*1.5f);
	
	error->color = manager->col_info_g;
	error->no_color = false;
		
	error->makeBox(width, height);
	//LoadPanel();
	//bkg = GetPointer("title_screen_bkg");
	
	//  OnActivate();
	FEGraphicalMenu::Init();
}
void GCMCDetectFrontEnd::goState(int which)
{
	char errorText[256];
	entries[MCOther]->Disable( true );
	last_state = mc_state;
	switch (which)
	{
	case MC_INSERT_CARD:
		error->changeText( GenericGameSaver::inst()->getErrorString( 0, 0, GSErrorNoMedia ) );
		error->makeBox(width, height);
		
		entries[MCContinue]->SetText(ksGlobalTextArray[GT_MC_CONTINUE]);
		entries[MCRetry]->SetText(ksGlobalTextArray[GT_MC_RETRY]);
		
		entries[MCContinue]->Disable(false);
		entries[MCRetry]->Disable(false);

		setHigh( entries[MCRetry], true );
		
		mc_state = MC_INSERT_CARD;
		break;
		
	case MC_CORRUPT:
		error->changeText( GenericGameSaver::inst()->getErrorString( 0, 0, GSErrorUnformatted ) );
		error->makeBox( width, height );
		entries[MCContinue]->SetText(ksGlobalTextArray[GT_MC_CONTINUE]);
		entries[MCOther]->SetText( ksGlobalTextArray[GT_MC_FORMAT] );
		entries[MCRetry]->SetText(ksGlobalTextArray[GT_MC_RETRY]);
		entries[MCContinue]->Disable(false);
		entries[MCOther]->Disable( false );
		entries[MCRetry]->Disable(false);
		setHigh( entries[MCRetry], true );
		mc_state = MC_CORRUPT;
		break;

	case MC_REGION:
		error->changeText( GenericGameSaver::inst()->getErrorString( 0, 0, GSErrorWrongRegion ) );
		error->makeBox( width, height );
		entries[MCContinue]->SetText(ksGlobalTextArray[GT_MC_CONTINUE]);
		entries[MCRetry]->SetText(ksGlobalTextArray[GT_MC_RETRY]);
		entries[MCContinue]->Disable(false);
		entries[MCRetry]->Disable(false);
		entries[MCOther]->SetText( ksGlobalTextArray[GT_MC_FORMAT] );
		entries[MCOther]->Disable( false );
		setHigh( entries[MCRetry], true );
		mc_state = MC_REGION;
		break;

	case MC_DAMAGED:
		error->changeText( GenericGameSaver::inst()->getErrorString( 0, 0, GSErrorDamaged ) );
		error->makeBox( width, height );
		entries[MCContinue]->SetText(ksGlobalTextArray[GT_MC_CONTINUE]);
		entries[MCRetry]->SetText(ksGlobalTextArray[GT_MC_RETRY]);
		entries[MCContinue]->Disable(false);
		entries[MCRetry]->Disable(false);
		setHigh( entries[MCRetry], true );
		mc_state = MC_DAMAGED;
		break;

	case MC_WRONG:
		error->changeText( GenericGameSaver::inst()->getErrorString( 0, 0, GSErrorUnknownMedia ) );
		error->makeBox( width, height );
		entries[MCContinue]->SetText(ksGlobalTextArray[GT_MC_CONTINUE]);
		entries[MCRetry]->SetText(ksGlobalTextArray[GT_MC_RETRY]);
		entries[MCContinue]->Disable(false);
		entries[MCRetry]->Disable(false);
		setHigh( entries[MCRetry], true );
		mc_state = MC_WRONG;
		break;

	case MC_COMPAT:
		error->changeText( GenericGameSaver::inst()->getErrorString( 0, 0, GSErrorIncompatible ) );
		error->makeBox( width, height );
		entries[MCContinue]->SetText(ksGlobalTextArray[GT_MC_CONTINUE]);
		entries[MCRetry]->SetText(ksGlobalTextArray[GT_MC_RETRY]);
		entries[MCContinue]->Disable(false);
		entries[MCRetry]->Disable(false);
		setHigh( entries[MCRetry], true );
		mc_state = MC_COMPAT;
		break;
	
	case MC_FORMAT_CARD:
		sprintf( errorText, ksGlobalTextArray[ GT_MC_FORMAT_GC ].c_str(), 'A' );
		error->changeText( errorText );
		error->makeBox( width, height );
		entries[MCContinue]->SetText(ksGlobalTextArray[GT_FE_MENU_YES]);
		entries[MCRetry]->SetText(ksGlobalTextArray[GT_FE_MENU_NO]);
		entries[MCContinue]->Disable(false);
		entries[MCRetry]->Disable(false);
		setHigh( entries[MCRetry], true );
		mc_state = MC_FORMAT_CARD;
		break;
		
	case MC_NO_DISK_SPACE:
		error->changeText(GenericGameSaver::inst()->getErrorString( 0, 0, GSErrorNotEnoughSpace ));
		error->makeBox(width, height);
		
		entries[MCContinue]->SetText(ksGlobalTextArray[GT_MC_CONTINUE]);
		entries[MCRetry]->SetText(ksGlobalTextArray[GT_MC_RETRY]);
		entries[MCOther]->SetText(ksGlobalTextArray[GT_MC_MANAGE]);
		
		entries[MCOther]->Disable(false);
		entries[MCContinue]->Disable(false);
		entries[MCRetry]->Disable(false);
		
		setHigh( entries[MCRetry], true );
		
		mc_state = MC_NO_DISK_SPACE;
		break;
		
	case MC_FORMATTING:
		sprintf( errorText, ksGlobalTextArray[ GT_MC_FORMATTING_GC ].c_str(), 'A' );
		error->changeText( errorText );
		error->makeBox( width, height );
		mc_state = MC_FORMATTING;
		entries[MCContinue]->Disable( true );
		entries[MCRetry]->Disable( true );
		format_count = 0;
		break;
	
	case MC_LOADING:
		error->changeText(GenericGameSaver::inst()->getLoadingString(savePort, saveSlot, ksGlobalTextArray[GT_MC_GAME_DATA])+ stringx("... ") );
		error->makeBox( width, height );
		entries[MCContinue]->Disable( true );
		entries[MCRetry]->Disable( true );
		mc_state = MC_LOADING;
		break;
		
	case MC_ERROR_LOADING:
		sprintf(errorText, ksGlobalTextArray[GT_MC_ERROR_LOADING].c_str(), ksGlobalTextArray[GT_MC_GAME_DATA].c_str(),  GenericGameSaver::inst()->getCardString(savePort, saveSlot).c_str());	
		error->changeText( errorText );
		error->makeBox( width, height );
		entries[MCContinue]->SetText(ksGlobalTextArray[GT_MC_CONTINUE]);
		entries[MCRetry]->SetText(ksGlobalTextArray[GT_MC_RETRY]);
		entries[MCContinue]->Disable( false );
		entries[MCRetry]->Disable( false );
		mc_state = MC_ERROR_LOADING;
		break;
		
	case MC_EXIT:
		mc_state = MC_EXIT;
		system->MakeActive(GraphicalMenuSystem::MainMenu);
		break;
	}
	
}

bool GCMCDetectFrontEnd::drawMenu()
{
	if( mc_state == MC_EXIT )
		return false;
	
	return true;
}

void GCMCDetectFrontEnd::Draw()
{
	if (drawMenu()) 
	{
		//bkg->Draw(0);
		error->Draw();
		FEGraphicalMenu::Draw();
	}
}

void GCMCDetectFrontEnd::Update(time_value_t time_inc)
{
	
	if( mc_state == MC_FORMATTING && format_count > 5 )
	{
		if (GenericGameSaver::inst()->format(0, 0) != GSOk)
			goState(MC_DAMAGED);
		else
			checkForErrors();  // SUCCESS
	}
	format_count++;
	
	if( mc_state == MC_LOADING && last_state == MC_LOADING )
		loadGlobalData();
	
	FEMultiMenu::Update(time_inc);
}

int GCMCDetectFrontEnd::validCards()
{
	struct
	{
		int status;
		int free;
	} card_stat[2];
	int dummy1, dummy2;
	
	card_stat[0].status = GenericGameSaver::inst()->getInfo( 0, 0, &dummy1, &(card_stat[0].free), &dummy2 );
	card_stat[1].status = GenericGameSaver::inst()->getInfo( 1, 0, &dummy1, &(card_stat[1].free), &dummy2 );

	if( card_stat[0].status == GSOk && GenericGameSaver::inst()->getFileListing( 0, 0, NULL, NULL, NULL ) == 0 )
	{
		if( card_stat[0].free < GenericGameSaver::inst()->getSavedGameSize() )
			card_stat[0].status = GSErrorNotEnoughSpace;
	}

	if( card_stat[1].status == GSOk && GenericGameSaver::inst()->getFileListing( 1, 0, NULL, NULL, NULL ) == 0 )
	{
		if( card_stat[1].free < GenericGameSaver::inst()->getSavedGameSize() )
			card_stat[1].status = GSErrorNotEnoughSpace;
	}

	// MemCard Guidelines, Table 4, (1)
	if( card_stat[0].status == GSOk
			&& card_stat[1].status == GSOk )
		return 2;

	// MemCard Guidelines, Table 4, (2, 3, 4, 7)
	if( card_stat[0].status == GSOk
			|| card_stat[1].status == GSOk )
		return 1;

	// MemCard Guidelines, Table 4, (5, 6, 8, 9)
	return card_stat[0].status;
}

int GCMCDetectFrontEnd::checkForErrors()
{
	int cards = validCards();
	
	if( cards > 0 )
		loadMostRecentGame();
	
	switch (cards)
	{
		case GSErrorNoMedia:
			goState(MC_INSERT_CARD);
			return cards;
			break;
		case GSErrorNotEnoughSpace:
			goState(MC_NO_DISK_SPACE);
			return cards;
			break;
		case GSErrorUnformatted:
			goState( MC_CORRUPT );
			return cards;
			break;
		case GSErrorUnknownMedia:
			goState( MC_WRONG );
			return cards;
			break;
		case GSErrorWrongRegion:
			goState( MC_REGION );
			return cards;
			break;
		case GSErrorDamaged:
			goState( MC_DAMAGED );
			return cards;
			break;
		case GSErrorIncompatible:
			goState( MC_COMPAT );
			return cards;
			break;
		default:
			break;
	}
	return cards;
}

void GCMCDetectFrontEnd::configLoadCallback(void *userData, int percent)
{
	if (percent >= 100)
	{
		((GCMCDetectFrontEnd *)userData)->goState(MC_LOADING);
		currentGame.valid = GSOk;
	}
	else if (percent < 0)
	{
		g_career->init();
		globalCareerData.init();
		((GCMCDetectFrontEnd *)userData)->goState(MC_ERROR_LOADING);
	}
}

void GCMCDetectFrontEnd::globalLoadCallback(void *userData, int percent)
{
	if (percent >= 100)
	{
		((GCMCDetectFrontEnd *)userData)->goState(MC_EXIT);
		globalCareerData.updateFromCareer(g_career);
	}
	else if (percent < 0)
	{
		globalCareerData.init();
		globalCareerData.updateFromCareer(g_career);
		((GCMCDetectFrontEnd *)userData)->goState(MC_ERROR_LOADING);
	}
}

bool GCMCDetectFrontEnd::findMostRecentGame(int &savePort, int &saveSlot, saveInfo &mostRecent)
{
	// Try to find an actual game to load
	int port, retVal = GSErrorOther;
	int type, free, formatted, oldestDate = 0;
	saveInfo SavedData[20];
	mostRecent.valid = GSErrorOther;
	
	saveSlot = 0;
	
	for(port = 0; port < NUM_MEMORY_PORTS; port++)
	{	
		retVal = GenericGameSaver::inst()->getInfo( port, 0, &type, &free, &formatted );

		if (retVal == GSOk)
		{
			int numGames = GenericGameSaver::inst()->getFileListing( port, 0, SavedData, NULL, NULL );
			for (int i=0; i < numGames; i++)
			{
				if (SavedData[i].valid == GSOk)
				{
					if (SavedData[i].timestamp > oldestDate)
					{
						oldestDate = SavedData[i].timestamp;
						savePort = port;
							
						memcpy( &mostRecent, &(SavedData[i]), sizeof(saveInfo) );
					}
				}
			}
		}
	}

	if (mostRecent.valid == GSOk)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool GCMCDetectFrontEnd::findGlobalData(int &foundPort, int &foundSlot)
{
	int port, slot;
	if( GenericGameSaver::inst()->hasSystemFile( foundPort, foundSlot ) )
	{
		return true;
	}
	return false;
}



int GCMCDetectFrontEnd::loadGlobalData()
{
	if( findGlobalData( savePort, saveSlot ) )
	{
		GenericGameSaver::inst()->readSystemFile( savePort, saveSlot, &globalCareerData, GCMCDetectFrontEnd::globalLoadCallback, this);			
		return GSOk;
	}
	else
		goState(MC_EXIT);

	return GSErrorOther;
}

int GCMCDetectFrontEnd::loadMostRecentGame()
{
	saveInfo  mostRecent;
	mostRecent.valid = -1;
	
	findMostRecentGame( savePort, saveSlot, mostRecent );

	// Now that we're done, check how many valid cards
	if( mostRecent.valid != GSOk )
	{
		g_career->init();
		globalCareerData.init();
		goState( MC_EXIT );
		return GSOk;
	}
	else 
	{
		foundGame = true; 
		memcpy( &currentGame, &mostRecent, sizeof(saveInfo) );
		GenericGameSaver::inst()->setFileInfo(mostRecent);
		goState( MC_LOADING );
		GenericGameSaver::inst()->readFile( savePort,saveSlot,g_career,sizeof(Career), GCMCDetectFrontEnd::configLoadCallback, this );
		return GSOk;
	}
	
}


void GCMCDetectFrontEnd::Select(int entry_index)
{
	if (mc_state == MC_FORMAT_CARD)  // IN THE FORMAT SCREEN
	{
		if (entry_index == MCContinue)
		{
			goState( MC_FORMATTING );
		}
		else
		{
			checkForErrors();
		}
	}
	else if (mc_state == MC_NO_DISK_SPACE)
	{
		if (entry_index == MCRetry)
		{
			checkForErrors();
		}
		else if( entry_index == MCOther )
		{
			extern void KSGCReset( bool ipl );
			KSGCReset( true ); // reboot to ipl
		}
		else  // CONTINUE
		{
			goState( MC_EXIT );
		}
	}
	else
	{
		if (entry_index == MCContinue)
		{
			goState(MC_EXIT);
		}
		else if( entry_index == MCOther ) //assume format
		{
			assert( mc_state == MC_REGION || mc_state == MC_CORRUPT );
			goState( MC_FORMAT_CARD );
		}
		else
		{
			checkForErrors();		
		}
	}
}

void GCMCDetectFrontEnd::OnActivate()
{
	setHigh(entries[MCRetry],true);
	checkForErrors();
}

void GCMCDetectFrontEnd::SetSystem(FEMenuSystem *s)
{
	system = s;
}
