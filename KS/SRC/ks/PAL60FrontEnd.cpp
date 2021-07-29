#include "PAL60FrontEnd.h"

PAL60FrontEnd::PAL60FrontEnd( FEMenuSystem* sys, FEManager* man, stringx p, stringx pq )
	: box( NULL ), msg( NULL ), yes( NULL ), no( NULL ), selected( 0 ), done( false )
{
	cons( sys, man, p, pq );

	flags |= FEMENU_DONT_SHOW_DISABLED;

	yes = NEW FEMenuEntry( ksGlobalTextArray[GT_FE_MENU_YES], this, false, &man->font_bold );
	Add( yes );
	no = NEW FEMenuEntry( ksGlobalTextArray[GT_FE_MENU_NO], this, false, &man->font_bold );
	Add( no );
	
	// utility slots, used by render_pal60_screen (game.cpp)
	first = yes;
	last = no;
}

PAL60FrontEnd::~PAL60FrontEnd( )
{
	delete msg;
	delete box;
}

void PAL60FrontEnd::Init( void )
{
	int w = nglGetScreenWidth( );
	int h = nglGetScreenHeight( );

	int width = w / 2;
	int height = h / 2;

	int left = width - ( width / 2 );
	int top = height - ( height / 2 );

	// I should go to Hell for this sort of buggery.
	yes->SetPos( left + ( width / 4 ), h - top - ( height / 4 ) );
	yes->right = no;
	no->SetPos( w - left - ( width / 4 ), h - top - ( height / 4 ) );
	no->left = yes;

	msg = NEW BoxText( &manager->font_bold_old,
										 ksGlobalTextArray[GT_FE_MENU_PAL60],
										 width,
										 height - ( height / 4 ) );
	msg->color = manager->white;
	// gutter space
	msg->makeBox( width - 15, height - 15 );

	box = NEW PanelQuad( "box_pal60" );
	// FIXME: make wider
	box->Init( left, top, left + width, top + height,
						 0.0f, 0.0f, 1.0f, 0.75f,
						 0.0f, 0.0f,
						 0.0f, 0.0f,
						 50.0f,
						 identity_matrix );

	FEGraphicalMenu::Init( );
}

void PAL60FrontEnd::Draw( void )
{
	box->Draw( 0 );
	msg->Draw( );
	FEGraphicalMenu::Draw( );
} 

void PAL60FrontEnd::Update( time_value_t delta )
{

	// If an entry has been selected, let's wait
	// until we've displayed the subsequent message
	// for a bit.
	if( selected ) {

		if( selected > 60 ) {
			done = true;
		}

		++selected;
	}

	FEMultiMenu::Update( delta );
}

void PAL60FrontEnd::Select( int idx )
{
	selected = 1;

#ifdef TARGET_GC
	if( idx == 0 ) {
		nglSetTVMode( NGLTV_EURBG60 );
		nglResetDisplay( );
		OSSetEuRgb60Mode( TRUE );

		msg->changeText( ksGlobalTextArray[GT_FE_MENU_PAL60_YES] );
	} else {
		OSSetEuRgb60Mode( FALSE );

		msg->changeText( ksGlobalTextArray[GT_FE_MENU_PAL60_NO] );
	}
#endif

	yes->Disable( true );
	no->Disable( true );

	int w = nglGetScreenWidth( ) / 2;
	int h = nglGetScreenHeight( ) / 2;
	msg->makeBox( w - 15, h - 15 );
}

void PAL60FrontEnd::OnActivate( void )
{
	setHigh( yes, true );
}

bool PAL60FrontEnd::IsDone( void )
{
	return done;
}