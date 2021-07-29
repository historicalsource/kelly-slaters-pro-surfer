
#include "global.h"
#include "igo_widget_fanmeter.h"

//	FanMeterWidget()
// Default constructor.
FanMeterWidget::FanMeterWidget()
{
	int	i;
	
	numSections = 24;
	sectionPQs = NEW PanelQuad4 *[numSections];
	for (i = 0; i < numSections; i++)
		sectionPQs[i] = NULL;

	arrowPQ = NULL;

	size = 1.0f;
	centerX = 0;
	centerY = 0;
}

//	~FanMeterWidget()
// Destructor.
FanMeterWidget::~FanMeterWidget()
{
	delete [] sectionPQs;
}

//	SetDisplay()
// Overridden from base class.
void FanMeterWidget::SetDisplay(const bool d)
{
	IGOWidget::SetDisplay(d);
}

//	Init()
// Called well after the constructor.
void FanMeterWidget::Init(PanelFile & panel)
{
	stringx	s;
	stringx	numStr;
	bool	firstInitialization = false;
	int		i;
	float	cx, cy;
	/*
	int		i, j;
	bool	foundCenter = false;
	*/

	if (sectionPQs[0] == NULL)
		firstInitialization = true;
	
	// Get pointers for each section.
	for (i = 0; i < numSections; i++)
	{
		// Build string of panel's name.
		s = "igo_tobw_";
		if (i < numSections/2)
		{
			s += "left_";
			numStr = stringx(numSections/2-i);
			if (numStr.size() == 1)
				numStr = "0" + numStr;
			s += numStr;
		}
		else
		{
			s += "right_";
			numStr = stringx(i-numSections/2+1);
			if (numStr.size() == 1)
				numStr = "0" + numStr;
			s += numStr;
		}

		// Get panel pointer.
		sectionPQs[i] = (PanelQuad4 *) panel.GetPointer(s.c_str());
	}
	
	arrowPQ = panel.GetPointer("igo_tobw_pointer");
	arrowPQ->TurnOn(true);
	if (firstInitialization)
	{
		arrowPQ->GetCenterPos(cx, cy);
		arrowPQ->SetCenterPos(cx, cy+15.0f);
	}

	// Perfect circle in XBox != perfect circle in PS2.
	if (firstInitialization)
	{
		#ifdef TARGET_PS2

		float	x[4], y[4];

		for (i = 0; i < numSections; i++)
		{
			x[0] = sectionPQs[i]->getQuad()->Verts[0].X;
			x[1] = sectionPQs[i]->getQuad()->Verts[1].X;
			x[2] = sectionPQs[i]->getQuad()->Verts[2].X;
			x[3] = sectionPQs[i]->getQuad()->Verts[3].X;
			y[0] = sectionPQs[i]->getQuad()->Verts[0].Y;
			y[1] = sectionPQs[i]->getQuad()->Verts[1].Y;
			y[2] = sectionPQs[i]->getQuad()->Verts[2].Y;
			y[3] = sectionPQs[i]->getQuad()->Verts[3].Y;
			unadjustCoords(x[0], y[0]);
			unadjustCoords(x[1], y[1]);
			unadjustCoords(x[2], y[2]);
			unadjustCoords(x[3], y[3]);
			x[0] = (x[0] - 320.0f)*0.80f + 256.0f;
			x[1] = (x[1] - 320.0f)*0.80f + 256.0f;
			x[2] = (x[2] - 320.0f)*0.80f + 256.0f;
			x[3] = (x[3] - 320.0f)*0.80f + 256.0f;
			y[0] = (y[0] - 240.0f)*0.80f + 224.0f;
			y[1] = (y[1] - 240.0f)*0.80f + 224.0f;
			y[2] = (y[2] - 240.0f)*0.80f + 224.0f;
			y[3] = (y[3] - 240.0f)*0.80f + 224.0f;

			sectionPQs[i]->SetVertices(x, y);
		}
		/*
		

		float	innerRadius = 15000.0f;
		float	outerRadius = 0.0f;
		float	theta = PI/float(numSections);

		// Get inner and outer radii.
		for (i = 0; i < 4; i++)
		{
			if (fabs(sectionPQs[0]->getQuad()->Verts[i].Y - 224.0f) <= 1.0f)
			{
				if (fabs(sectionPQs[0]->getQuad()->Verts[i].X - 256.0f) < innerRadius)
					innerRadius = fabs(sectionPQs[0]->getQuad()->Verts[i].X - 256.0f);
				if (fabs(sectionPQs[0]->getQuad()->Verts[i].X - 256.0f) > outerRadius)
					outerRadius = fabs(sectionPQs[0]->getQuad()->Verts[i].X - 256.0f);
			}
		}

		// Set all section vertices to a pretty circle.
		for (i = 0; i < numSections; i++)
		{
			// Rotate them.  (note: verts in 3D studio must match!)
			x[2] = 256.0f + innerRadius*cosf((numSections-i)*theta);
			x[3] = 256.0f + outerRadius*cosf((numSections-i)*theta);
			x[1] = 256.0f + outerRadius*cosf((numSections-1-i)*theta);
			x[0] = 256.0f + innerRadius*cosf((numSections-1-i)*theta);
			y[2] = 224.0f - innerRadius*sinf((numSections-i)*theta);
			y[3] = 224.0f - outerRadius*sinf((numSections-i)*theta);
			y[1] = 224.0f - outerRadius*sinf((numSections-1-i)*theta);
			y[0] = 224.0f - innerRadius*sinf((numSections-1-i)*theta);

			// Give new coords back to quad.
			sectionPQs[i]->SetVertices(x, y);
		}

		*/
		#endif
	}

	// Extract the center rotation point.
	/*
	for (i = 0; i < numSections; i++)
	{
		for (j = i+1; j < numSections; j++)
		{
			if (sectionPQs[0]->getQuad()->Verts[i].X == sectionPQs[0]->getQuad()->Verts[j].X &&
				sectionPQs[0]->getQuad()->Verts[i].Y == sectionPQs[0]->getQuad()->Verts[j].Y)
			{
				centerX = int(sectionPQs[0]->getQuad()->Verts[i].X);
				centerY = int(sectionPQs[0]->getQuad()->Verts[i].Y);
				unadjustCoords(centerX, centerY);
				foundCenter = true;
				break;
			}
		}
		if (foundCenter) break;
	}
	*/
	centerX = 320;
	centerY = 240;

	SetDisplay(true);
	Show(false);
	angle = 0;
}

//	Update()
// Call every frame - update widget with time elapsed since last frame.
void FanMeterWidget::Update(const float dt)
{
	IGOWidget::Update(dt);
}

//	Draw()
// Sends the widget's quads to NGL.
void FanMeterWidget::Draw(void)
{
	int	i;
	
	IGOWidget::Draw();

	if (!display)
		return;

	for (i = 0; i < numSections; i++)
		sectionPQs[i]->Draw(0);
	arrowPQ->Draw(0);
}

//	Show()
// Turns the meter on/off.
void FanMeterWidget::Show(const bool s)
{
	int	i;
	
	if (s)
	{
		SetSize(size);
		arrowPQ->TurnOn(true);
	}
	else
	{
		for (i = 0; i < numSections; i++)
			sectionPQs[i]->TurnOn(false);
		arrowPQ->TurnOn(false);
	}
}

//	SetArrowPos()
// Sets the position of the pointer arrow. [-1, 1]
void FanMeterWidget::SetArrowPos(float f)
{
	if (f < -1.0f)
		f = -1.0f;
	else if (f > 1.0f)
		f = 1.0f;

	arrowPQ->Rotate(centerX, centerY, f*PI/2.0f);
}

//	SetSize()
// Sets how open the fan is. (0, 1)
void FanMeterWidget::SetSize(const float f)
{	
	float	pos = 0.0, dt = 0.0f;
	int		numHoles;
	int		dest = (numSections%2 == 0) ? (numSections/2-1) : (numSections/2);
	int		i, j;

	for (i = 0; i < numSections; i++)
		sectionPQs[i]->TurnOn(false);

	if (f > 0.0f)
		dt = 1.0f/f;

	while (dt > 0.0f && int(pos) <= dest)
	{
		sectionPQs[int(pos)]->TurnOn(true);
		pos += dt;
	}
	for (i = numSections-1; i > dest; i--)
	{
		sectionPQs[i]->TurnOn(sectionPQs[numSections-1-i]->IsOn());
	}

	// Rotate left selected regions.
	for (i = 0; i < numSections/2; i++)
	{
		numHoles = 0;
		for (j = i+1; j < numSections/2; j++)
		{
			if (!sectionPQs[j]->IsOn())
				numHoles++;
		}

		sectionPQs[i]->Rotate(centerX, centerY, float(numHoles)*(PI/float(numSections)));
	}

	// Rotate right selected regions.
	for (i = numSections-1; i >= numSections/2; i--)
	{
		numHoles = 0;
		for (j = i-1; j >= numSections/2; j--)
		{
			if (!sectionPQs[j]->IsOn())
				numHoles++;
		}

		sectionPQs[i]->Rotate(centerX, centerY, float(numHoles)*(-PI/float(numSections)));
	}

	// Hack - fix missing center pixel.
	/*
	int	l = numSections/2-1;
	int r = numSections/2;
	while (l > 0 && !sectionPQs[l]->IsOn()) l--;
	while (r < numSections-1 && !sectionPQs[r]->IsOn()) r++;
	if (fabs(sectionPQs[l]->getQuad()->Verts[1].X - sectionPQs[r]->getQuad()->Verts[3].X) >= 1.0f)
	{
		sectionPQs[l]->Rotate(centerX, centerY, sectionPQs[l]->GetRotation()+0.017f);
	}
	*/

	size = f;
}

/*
//	SelectSections()
// Private helper function - recursive method that selects num sections
// between l and r.
void FanMeterWidget::SelectSections(const int l, const int r, const int num)
{
	if (num >= 1)
	{
		// If we have odd-numbered sections to divide...
		if (((r-l+1)%2 == 1) && (r-l >= 1))
		{
			// Turn on the section in the middle.
			sectionPQs[l+(r-l)/2]->TurnOn(true);

			// Subdivide the two new regions.
			SelectSections(l, l+(r-l)/2, num/2);
			SelectSections(l+(r-l)/2, r, num/2);
		}
		// If we have even-numbered sections to divide...
		else if (((r-l+1)%2 == 0) && (r-l >= 2))
		{
			// Turn on the two sections in the middle.
			sectionPQs[l+(r-l+1)/2-1]->TurnOn(true);
			sectionPQs[l+(r-l+1)/2]->TurnOn(true);

			// Subdivide the two new regions.
			SelectSections(l+(r-l+1)/2, r, num/2-1);
			SelectSections(l, l+(r-l+1)/2-1, num/2-1);
			
		}
	}
}
*/