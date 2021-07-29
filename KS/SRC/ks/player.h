
#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "entity.h"
#include "billboard.h"
#include "surferdata.h"	// For SURFER_LAST

enum
{
	ATTRIB_AIR,
	ATTRIB_SPEED,
	ATTRIB_ROTATE,
	ATTRIB_BALANCE,
	NUM_ATTRIBS
};

/*	Use enum in surferdata.h instead.  (dc 01/16/02)
enum
{
	KELLY_SLATER_ATTR,
	ROB_MACHADO_ATTR,
	KALANI_ROBB_ATTR,
	DONAVAN_FRANKENREITER_ATTR,
	TOM_CURREN_ATTR,
	TOM_CARROL_ATTR,
	LISA_ANDERSEN_ATTR,
	BRUCE_IRONS_ATTR,
	NATHAN_FLETCHER_ATTR,
	PASTRANA_ATTR,
	PERSONALITYKS_ATTR,
	TONY_HAWK_ATTR,
	KEALA_KENNELLY_ATTR,
	SILVER_SURFER_ATTR,
	DOG_ATTR,
	NUM_SURFER_ATTRS
};
*/

enum
{
	BOARD_1_ATTR,
	BOARD_2_ATTR,
	NUM_BOARDS_ATTRS
};


class SurferAttributes
{
public:

	SurferAttributes();

	int player_rotate;
	int player_speed;
	int player_balance;
	int player_air;

	int board_rotate;
	int board_speed;
	int board_balance;
	int board_air;
	
	int use_debug_values;	// 0 or 1, changed from bool so it will be compatible with debug menu

	int player_attribs[SURFER_LAST][NUM_ATTRIBS];
	int board_attribs[NUM_BOARDS_ATTRS][NUM_ATTRIBS];
};


class player_info
{
public:
	enum { MAX_ACC = 20 };

protected:
	entity *	accomplishments[MAX_ACC];
	int			numAccomplishments;

public:
	player_info();
	~player_info();

	void AddAccomplishment(entity * ent);
	void ClearAccomplishments(void);
	int GetNumAccomplishments(void) const { return numAccomplishments; }
	bool HasAccomplishment(const entity * ent) const;
};

extern player_info current_player_info;
extern SurferAttributes g_surfer_attribs;

extern bool TakePhoto (float dt, void** data);

#endif // _PLAYER_H_ 
