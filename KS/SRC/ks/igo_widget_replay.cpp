
#include "global.h"
#include "igo_widget_replay.h"

//	ReplayWidget()
// Default constructor.
ReplayWidget::ReplayWidget()
{
  vcrButton       = VCR_PLAY;
  vcrButtonHL     = VCR_PLAY;

  vcrPQ             = NULL;
  vcrHLPQ           = NULL;
  vcrBGPQ           = NULL;
  restartPQ         = NULL;
  restartOffPQ      = NULL;
  restartHLPQ       = NULL;
  pausePQ           = NULL;
  pauseOffPQ        = NULL;
  pauseHLPQ         = NULL;
  playPQ            = NULL;
  playOffPQ         = NULL;
  playHLPQ          = NULL;
  slowPQ            = NULL;
  slowOffPQ         = NULL;
  slowHLPQ          = NULL;
  fastforwardPQ     = NULL;
  fastforwardOffPQ  = NULL;
  fastforwardHLPQ   = NULL;

  int tx = 320, ty = 400;
  //unadjustSizes(tx, ty);
  pauseText = NEW TextString(NULL, ksGlobalTextArray[GT_REPLAY_PAUSED], tx, ty, 800, 1.0f, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, frontendmanager.col_highlight);
}

//	~ReplayWidget()
// Destructor.
ReplayWidget::~ReplayWidget()
{
  delete pauseText;
}

//	SetDisplay()
// Overridden from base class.
void ReplayWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
}

//	Init()
// Called well after the constructor.
void ReplayWidget::Init(PanelFile & panel, Font *font)
{
	vcrPQ             = panel.GetPointer("replay_main2");
	vcrHLPQ           = panel.GetPointer("replay_main3");
	vcrBGPQ           = panel.GetPointer("replay_main4");
	restartPQ         = panel.GetPointer("replay_restart");
	restartOffPQ      = panel.GetPointer("replay_restart_off");
	restartHLPQ       = panel.GetPointer("replay_restart_hilite");
	pausePQ           = panel.GetPointer("replay_pause");
	pauseOffPQ        = panel.GetPointer("replay_pause_off");
	pauseHLPQ         = panel.GetPointer("replay_pause_hilite");
	playPQ            = panel.GetPointer("replay_play");
	playOffPQ         = panel.GetPointer("replay_play_off");
	playHLPQ          = panel.GetPointer("replay_play_hilite");
	slowPQ            = panel.GetPointer("replay_slowforward");
	slowOffPQ         = panel.GetPointer("replay_slowforward_off");
	slowHLPQ          = panel.GetPointer("replay_slowforward_hilite");
	fastforwardPQ     = panel.GetPointer("replay_fastforward");
	fastforwardOffPQ  = panel.GetPointer("replay_fastforward_off");
	fastforwardHLPQ   = panel.GetPointer("replay_fastforward_hilite");

  pauseText->setFont(font);

  highlight_intensity = 0.0f;
  highlight_timer     = 0.0f;
}

//	Update()
// Call every frame - update widget with time elapsed since last frame.
void ReplayWidget::Update(const float dt)
{
	IGOWidget::Update(dt);

	float period = 
		(float)(os_developer_options::inst()->get_int(os_developer_options::INT_MENU_HIGHLIGHT_PERIOD)) / 
		MENU_HIGHLIGHT_MULTIPLIER;

  highlight_timer += dt;
  highlight_intensity = THROB_INTENSITY * sinf(2*PI*(highlight_timer/period));

  static color32 c   = color32(224, 128, 0, 255);
  static color32 ch  = color32(255, 255, 0, 255);

  int r = FTOI(c.get_red() + (highlight_intensity+.5f)*(ch.get_red() - c.get_red()));
  int g = FTOI(c.get_green() + (highlight_intensity+.5f)*(ch.get_green() - c.get_green()));
  int b = FTOI(c.get_blue() + (highlight_intensity+.5f)*(ch.get_blue() - c.get_blue()));
  int a = 255;

  //int r = FTOI(160.0f + 95.0f*highlight_intensity);
  //int g = FTOI(160.0f + 95.0f*highlight_intensity);
  //int b = 64;

  pauseText->color = color32(r, g, b, a);
}

//	Draw()
// Sends the widget's quads to NGL.
void ReplayWidget::Draw(void)
{
	IGOWidget::Draw();

	if (!display)
		return;

	vcrPQ->Draw(0);
	vcrHLPQ->Draw(0);
	vcrBGPQ->Draw(0);
	restartOffPQ->Draw(0);
	pauseOffPQ->Draw(0);
	playOffPQ->Draw(0);
	slowOffPQ->Draw(0);
	fastforwardOffPQ->Draw(0);

  switch(vcrButton)
  {
    case VCR_RESTART:
      restartPQ->Draw(0);
      break;
    case VCR_PAUSE:
      pausePQ->Draw(0);
      pauseText->Draw();
      break;
    case VCR_PLAY:
      playPQ->Draw(0);
      break;
    case VCR_FASTFORWARD:
      fastforwardPQ->Draw(0);
      break;
    case VCR_SLOW:
      slowPQ->Draw(0);
      break;
  }

  switch(vcrButtonHL)
  {
    case VCR_RESTART:
      restartHLPQ->Draw(0);
      break;
    case VCR_PAUSE:
      pauseHLPQ->Draw(0);
      break;
    case VCR_PLAY:
      playHLPQ->Draw(0);
      break;
    case VCR_FASTFORWARD:
      fastforwardHLPQ->Draw(0);
      break;
    case VCR_SLOW:
      slowHLPQ->Draw(0);
      break;
  }
}

//  Select()
// Activates the specified button
void ReplayWidget::Select(int button)
{
  vcrButton = button;

  if(vcrButton < VCR_RESTART)
    vcrButton = VCR_RESTART;
  if(vcrButton > VCR_FASTFORWARD)
    vcrButton = VCR_FASTFORWARD;
}

//  SelectHighlight()
// Highlights the specified button
void ReplayWidget::SelectHighlight(int highlight)
{
  vcrButtonHL = highlight;

  if(vcrButtonHL < VCR_RESTART)
    vcrButtonHL = VCR_RESTART;
  if(vcrButtonHL > VCR_FASTFORWARD)
    vcrButtonHL = VCR_FASTFORWARD;
}

//  HighlightLeft()
// Highlights the button to the left of the current button
void ReplayWidget::HighlightLeft()
{
  vcrButtonHL--;
  if(vcrButtonHL < VCR_RESTART)
    vcrButtonHL = VCR_RESTART;
}

//  HighlightRight()
// Highlights the button to the right of the current button
void ReplayWidget::HighlightRight()
{
  vcrButtonHL++;
  if(vcrButtonHL > VCR_FASTFORWARD)
    vcrButtonHL = VCR_FASTFORWARD;
}
