#ifndef PAL60FRONTEND_H
#define PAL60FRONTEND_H

#include "FEMenu.h"

class PAL60FrontEnd : public FEMultiMenu
{
protected:
	PanelQuad* box;
	BoxText* msg;	
	FEMenuEntry* yes;
	FEMenuEntry* no;
	int selected;
	bool done;

public:
	PAL60FrontEnd( FEMenuSystem* sys, FEManager* man, stringx p, stringx pq );
	virtual ~PAL60FrontEnd( );

	virtual void Init( void );
	virtual void Draw( void );
	virtual void Update( time_value_t delta );
	virtual void Select( int idx );
	virtual void Pick( int idx ) { };
	virtual void OnActivate( void );
	virtual void OnTriangle( int c ) { };
	virtual void SetSystem( FEMenuSystem* s ) { system = s; };
	
	bool IsDone( void );
};

#endif