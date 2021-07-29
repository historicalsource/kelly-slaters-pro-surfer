
#include "global.h"
#include "igo_widget_iconradar.h"

//	IconRadarWidget()
// Default constructor.
IconRadarWidget::IconRadarWidget()
{
	int	i = 0;
	
	ellipsePQ = NULL;
	challenge = NULL;

	numIconTextures = IconChallenge::Task::TYPE_NUM;
	iconTextures = NEW nglTexture *[numIconTextures];
	for (i = 0; i < numIconTextures; i++)
		iconTextures[i] = NULL;

	numOnscreenIcons = 0;
	onscreenIcons = NEW OnscreenIcon[IconChallenge::MAX_SEQUENCE_SIZE];
}

//	~IconRadarWidget()
// Destructor.
IconRadarWidget::~IconRadarWidget()
{
	delete iconTextures;
	delete onscreenIcons;
}

//	Init()
// Must be called after constructing.
void IconRadarWidget::Init(PanelFile & panel, IconChallenge * chall)
{
	challenge = chall;
	
	ellipsePQ = panel.GetPointer("radar_ellipse");
	ellipsePQ->TurnOn(true);
	
	nglSetTexturePath("challenges\\ghosticon\\textures\\");
	iconTextures[IconChallenge::Task::TYPE_AIR_FLIP] = nglGetTexture("Flip");
	iconTextures[IconChallenge::Task::TYPE_AIR_GRAB] = nglGetTexture("Grab");
	iconTextures[IconChallenge::Task::TYPE_AIR_SPIN] = nglGetTexture("Spin");
	iconTextures[IconChallenge::Task::TYPE_AIR_POINTS_1000] = nglGetTexture("Points");
	iconTextures[IconChallenge::Task::TYPE_AIR_POINTS_2000] = nglGetTexture("Points");
	iconTextures[IconChallenge::Task::TYPE_AIR_POINTS_5000] = nglGetTexture("Points");
	iconTextures[IconChallenge::Task::TYPE_AIR_SPECIAL] = nglGetTexture("Special");
	iconTextures[IconChallenge::Task::TYPE_FACE_SPIN] = nglGetTexture("Spin");
	iconTextures[IconChallenge::Task::TYPE_TUBE_TIME_10] = nglGetTexture("Time");
	iconTextures[IconChallenge::Task::TYPE_TUBE_TIME_15] = nglGetTexture("Time");
	iconTextures[IconChallenge::Task::TYPE_TUBE_TIME_30] = nglGetTexture("Time");
	iconTextures[IconChallenge::Task::TYPE_TUBE_TRICK] = nglGetTexture("icon_Trick");

	SetDisplay(true);
}

//	Update()
// Called every frame with the time elapsed since the previous frame.
void IconRadarWidget::Update(const float dt)
{
	const IconChallenge::Sequence *	currSequence;
	nglMatrix						mat;
	nglVector						dest;
	int								i;
	
	IGOWidget::Update(dt);

	// Collect onscreen icon information.
	nglGetMatrix(mat, NGLMTX_WORLD_TO_SCREEN);
	numOnscreenIcons = 0;
	if (challenge && challenge->GetCurrentSequence())
	{
		// Add all uncompleted tasks in this sequence.
		currSequence = challenge->GetCurrentSequence();
		for (i = 0; i < currSequence->GetNumTasks(); i++)
		{
			// Add this task.
			if (!currSequence->GetTask(i)->IsCompleted())
			{
				onscreenIcons[numOnscreenIcons].texture = iconTextures[currSequence->GetTask(i)->GetType()];
				onscreenIcons[numOnscreenIcons].screenPos[0] = currSequence->GetTask(i)->GetPosition().x;
				onscreenIcons[numOnscreenIcons].screenPos[1] = currSequence->GetTask(i)->GetPosition().y;
				onscreenIcons[numOnscreenIcons].screenPos[2] = currSequence->GetTask(i)->GetPosition().z;
				onscreenIcons[numOnscreenIcons].screenPos[3] = 1;
				nglApplyMatrix(dest, mat, onscreenIcons[numOnscreenIcons].screenPos);
				onscreenIcons[numOnscreenIcons].screenPos = dest;
				unadjustCoords(onscreenIcons[numOnscreenIcons].screenPos[0], onscreenIcons[numOnscreenIcons].screenPos[1]);
				numOnscreenIcons++;
			}
		}
	}
}

//	Draw()
// Sends this widget's quads to NGL.
void IconRadarWidget::Draw(void)
{
	int		i;
	
	IGOWidget::Draw();

	if (!display)
		return;

	ellipsePQ->Draw(0);

	for (i = 0; i < numOnscreenIcons; i++)
	{
		
	}
}
