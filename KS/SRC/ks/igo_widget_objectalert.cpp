#include "global.h"
#include "igo_widget_objectalert.h"

const float ObjectAlertWidget::TIME_FADE = 0.30f;
const float ObjectAlertWidget::TIME_ANIMATE = 0.5f;
const float ObjectAlertWidget::SPEED_HILITE_FLASH = 2.0f;

ObjectAlertWidget::ObjectAlertWidget()
{
	objectRoot = NULL;

	fade = 0.0f;
	fadeDir = -1;

	display = false;
}

//	~ObjectAlertWidget()
// Destructor.
ObjectAlertWidget::~ObjectAlertWidget()
{
}

//	SetDisplay()
// Toggles this widget on/off.
void ObjectAlertWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
	display = d;
}

//	Init()
// Must be called after constructing.
void ObjectAlertWidget::Init(PanelFile & panel)
{
	objectRoot = panel.GetPointer("objectalert");

	SetDisplay(true);
	Hide();
	Update(0.0f);
}

//	Update()
// Called every frame with the time elapsed since the previous frame.
void ObjectAlertWidget::Update(const float dt)
{
	bool	object_near = false;
	
	IGOWidget::Update(dt);

	// Fade overlay in and out.
	fade += (dt/TIME_FADE)*float(fadeDir);
	if (fade >= 1.0f)
		fade = 1.0f;
	else if (fade <= 0.0f)
	{
		fade = 0.0f;
	}

	if ((fade <= 1.0f) && (fade >= 0.0f))
	{
		objectRoot->SetFade(fade);
	}

	if (FindNearestObject())
		object_near = true;

	if (object_near && (fade <= 1.0f))
	{
		//  This was just turned on, so explain what it is.
		if (fadeDir == -1 && 
			(g_world_ptr->get_ks_controller(g_game_ptr->get_active_player()))->get_super_state() != SUPER_STATE_WIPEOUT)
			frontendmanager.IGO->GetHintManager()->SetHint(IGOHintManager::ObjectApproaching);

		fadeDir = 1;
	}
	else if (!object_near)
		fadeDir = -1;
}

//	Draw()
// Sends this widget's quads to NGL.
void ObjectAlertWidget::Draw(void)
{
	IGOWidget::Draw();

	if (!display)
		return;

	// Photo widget overrides object alert widget.
	if (!frontendmanager.IGO->IsPhotoShown())
		objectRoot->Draw(0);
}

//	Show()
// Activates the wave indicator overlay.
void ObjectAlertWidget::Show(const bool fadeIn)
{
	fadeDir = 1;
	if (!fadeIn)
		fade = 1.0f;
}

//	Hide()
// Deactivates the wave indicator overlay.
void ObjectAlertWidget::Hide(const bool fadeOut)
{
	fadeDir = -1;
	if (!fadeOut)
		fade = 0.0f;
}

float g_objalert_dist_limit = 40000.0f;
bool ObjectAlertWidget::FindNearestObject(void)
{
	vector3d object_pos;
	int player = g_game_ptr->get_active_player();
	kellyslater_controller *ksctrl = g_world_ptr->get_ks_controller(player);
	vector3d lipnorm = ksctrl->get_board_controller().GetLipNormal();
	bool object_near = false;

	if (g_beach_ptr && g_beach_ptr->get_nearest_object_pos(ksctrl->get_my_board_model()->get_abs_position(), object_pos, lipnorm))
	{
		if (dot(object_pos, object_pos) < g_objalert_dist_limit)
		{
			object_near = true;
		}
		else
			object_near = false;
	}
	else
		object_near = false;

	return object_near;
}