// FEAnim.cpp

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "hwrasterize.h"

#include "FEAnim.h"
#include "kshooks.h"



bool PanelAnimKeyframe::Load(unsigned char* buffer, int& index)
{
	idx = ReadFloat(buffer, index);
	ReadVector3d(translation, buffer, index);
	ReadVector3d(euler, buffer, index);
	ReadVector3d(scale, buffer, index);
	event = ReadString(buffer, index);
	return true;
}

bool PanelAnim::Load(unsigned char* buffer, int& index, PanelFile* target_panel)
{
	name = ReadString(buffer, index);
	ReadMatrix3x4(matrix, buffer, index);
	numkeyframes = ReadLong(buffer, index);
	keyframes = NEW PanelAnimKeyframe[numkeyframes];

	for (u_int i=0; i<numkeyframes; i++)
	{
		PanelAnimKeyframe kf;
		if (!kf.Load(buffer, index))
			return false;
		keyframes[i] = kf;
	}

	uint32 nchildren = ReadLong(buffer, index);
	for (int i = nchildren; --i >= 0; )
	{
		PanelAnim* anim=PanelAnim::LoadPanelAnim(buffer, index, target_panel);
		if (!anim)
		  return false;

		anim->parent = this;
		anim->next = children;
		children = anim;
	}

	quad = target_panel ? target_panel->FindQuad(name) : NULL;
	if(quad) quad->MakeAnim(this);
	return true;
}

PanelAnim::~PanelAnim()
{
	delete[] keyframes;
	PanelAnim* tmp = children;
	PanelAnim* tmp2;
	while(tmp)
	{
		tmp2 = tmp->next;
		delete tmp;
		tmp = tmp2;
	}
}

PanelAnim* PanelAnim::LoadPanelAnim(unsigned char* buffer,
									int& index,
									PanelFile* target_panel)
{
#ifdef DEBUG
	uint8 type = ReadChar(buffer, index);
	assert(type == AnimInstance);
#else
	ReadChar(buffer, index);
#endif
	PanelAnimInstance* inst=NEW PanelAnimInstance;
	if (!inst->Load(buffer, index, target_panel))
	{
		delete inst;
		return NULL;
	}
	return inst;

}


//  Take a matrix from the parent panelanim, and use that to render this object.
void PanelAnim::Update(float time, const matrix4x4& parent_matrix)
{
	matrix4x4 Xform = GetXFormMatrix( time );

#if defined(TARGET_XBOX)
  assert(Xform.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

	Xform = Xform*parent_matrix;

#if defined(TARGET_XBOX)
  assert(Xform.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

	if(quad) quad->Xform = Xform;
	PanelAnim* kids = children;
	while (kids != NULL)
	{
		kids->Update(time, Xform);
		kids = kids->next;
	}
}

matrix4x4 PanelAnim::GetXFormMatrix(float time, bool is_fe_cam)
{

//  vector3d no_transform(0,0,0);
  matrix4x4 scale_matrix;
  matrix4x4 euler_matrix;

  if (keyframes)
  {
    const PanelAnimKeyframe& firstkey = keyframes[0];
    const PanelAnimKeyframe& lastkey = keyframes[numkeyframes - 1];

    //  If we're past the last keyframe's time, then just use the last keyframe's data
    if (time >= lastkey.idx)
    {
      scale_matrix.make_scale(lastkey.scale);
      make_euler(euler_matrix, lastkey.euler, is_fe_cam);
      scale_matrix = scale_matrix * euler_matrix;
      scale_matrix.w = vector4d(lastkey.translation.x, lastkey.translation.y, lastkey.translation.z, 1);
    }
    else if (time < firstkey.idx)  //  Special case for if the first keyframe is after the current time.
    {
//      float delta = time / firstkey.idx;

      scale_matrix.make_scale(firstkey.scale);
      make_euler(euler_matrix, firstkey.euler, is_fe_cam);
      scale_matrix = scale_matrix * euler_matrix;
      scale_matrix.w = vector4d(firstkey.translation.x, firstkey.translation.y, firstkey.translation.z, 1);
    }
    else  //  Figure out how far between frames we are and interpolate.
    {
      u_int currentkeyframe = 0;
      float starttime = keyframes[0].idx;

      //  Catch up to the keyframe that passed our current animation time last.
      while (currentkeyframe < numkeyframes - 1 &&
        keyframes[currentkeyframe + 1].idx - starttime < time - starttime)
        currentkeyframe++;

      const PanelAnimKeyframe& curkey = keyframes[currentkeyframe];
      const PanelAnimKeyframe& nextkey = keyframes[currentkeyframe+1];

      //  Figure out the current distance between frames.
#if defined(TARGET_XBOX)
      double delta = (time - curkey.idx) / (nextkey.idx - curkey.idx);
      assert( delta == delta );
#else
      float delta = (time - curkey.idx) / (nextkey.idx - curkey.idx);
#endif /* TARGET_XBOX JIV DEBUG */

      vector3d newscale = curkey.scale + ((nextkey.scale - curkey.scale) * delta);
      vector3d neweuler = curkey.euler + ((nextkey.euler - curkey.euler) * delta);
      vector3d newtranslation = curkey.translation + ((nextkey.translation - curkey.translation) * delta);

#if defined(TARGET_XBOX)
      assert( newscale == newscale );
      assert( neweuler == neweuler );
      assert( newtranslation == newtranslation );
#endif /* TARGET_XBOX JIV DEBUG */

      //  Make sure that the new euler hasn't gone out of useful range.
      float eulerz = nextkey.euler.z - curkey.euler.z;
      if (eulerz < -PI || eulerz > PI)
      {
        eulerz = fmod(eulerz,2*PI);

        if (eulerz < -PI)
          eulerz += 2*PI;
        else if (eulerz > PI)
          neweuler.z -= 2*PI;

#if defined(TARGET_XBOX)
      assert( eulerz == eulerz );
      assert( curkey.euler.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

        neweuler.z = curkey.euler.z + (eulerz * delta);

#if defined(TARGET_XBOX)
      assert( neweuler.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
      }

      scale_matrix.make_scale(newscale);
      make_euler(euler_matrix, neweuler, is_fe_cam);
      scale_matrix = scale_matrix * euler_matrix;

      scale_matrix.w = vector4d(newtranslation.x, newtranslation.y, newtranslation.z, 1);
    }
  }
  else
    scale_matrix = matrix;

#if defined(TARGET_XBOX)
  assert(scale_matrix.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

  return scale_matrix;

}


bool PanelAnim::GetEvent(float last_time, float current_time, stringx &event)
{
	event = "";

	if (keyframes)
	{
		const PanelAnimKeyframe& firstkey = keyframes[0];
		const PanelAnimKeyframe& lastkey = keyframes[numkeyframes - 1];

		//  If we're past the last keyframe's time or things haven't started yet, then no events were found
		if (last_time >= lastkey.idx || current_time <= firstkey.idx)
		{
			return false;
		}
		else  //  Figure out how far between frames we are and interpolate.
		{
			if (last_time < firstkey.idx)
				last_time = firstkey.idx;

			if (current_time > lastkey.idx)
				current_time = lastkey.idx;

			u_int currentkeyframe = 0;
			float starttime = keyframes[0].idx;

			//  Catch up to the keyframe that passed our current animation time last.
			while (currentkeyframe < numkeyframes - 1 &&
				keyframes[currentkeyframe + 1].idx - starttime < current_time - starttime)
				currentkeyframe++;

			const PanelAnimKeyframe& curkey = keyframes[currentkeyframe];

			//  Check to see if we've already caught this one.
			if (curkey.idx <= last_time)
				return false;

			event = curkey.event;
			return true;
		}
	}

	return false;
}


PanelAnim *PanelAnim::FindObject(char *object_name)
{
	//char tempname[80] = name.data;
	if (!strcmp(name.data(), object_name))
		return this;
	else if (next != NULL)
		return next->FindObject(object_name);
	else
		return NULL;
}


bool PanelAnimInstance::Load(unsigned char* buffer, int& index, PanelFile* target_panel)
{
	if (!PanelAnim::Load(buffer, index, target_panel))
		return false;
	filename = ReadString(buffer, index);
	return true;
}

/*
PanelAnimFile::PanelAnimFile()//char* filename, PanelFile* target_panel)
{
	obs = NULL;
	totalseconds = 0.0f;
//	Load(filename, target_panel);
}
*/
matrix4x4 PanelAnimFile::GetXFormMatrix(float time, bool is_fe_cam)
{
	return obs->GetXFormMatrix(time, is_fe_cam);
}

bool PanelAnimFile::Load(char* filename, PanelFile* target_panel)
{
	nglFileBuf file;
	file.Buf=NULL; file.Size=0;
	KSReadFile(filename, &file, 1);
//	int buf_size = file.Size;
	unsigned char* buffer = (unsigned char*) file.Buf;
	int index = 0;
	if(!buffer)
	{
		KSReleaseFile(&file);
		assert(0);
		return false;
	}
	if(!ReadHeader(buffer, index))
	{
		KSReleaseFile(&file);
		assert(0);
		return false;
	}
//	uint32 version = 
		ReadLong(buffer, index);
	totalseconds = ReadFloat(buffer, index);
	uint32 nobjects = ReadLong(buffer, index);

	for (u_int i=0; i<nobjects; ++i)
	{
		PanelAnim* anim = PanelAnim::LoadPanelAnim(buffer, index, target_panel);
		if (!anim)
		{
			KSReleaseFile(&file);
			assert(0);
			return false;
		}
		anim->next = obs;
		obs = anim;
	}

	KSReleaseFile(&file);
	return true;
}

PanelAnimFile::~PanelAnimFile()
{
	PanelAnim* tmp = obs;
	PanelAnim* tmp2;
	while(tmp)
	{
		tmp2 = tmp->next;
		delete tmp;
		tmp = tmp2;
	}
}

bool PanelAnimFile::ReadHeader(unsigned char* buffer, int& index)
{
	if(ReadChar(buffer, index) == 'A' &&
	   ReadChar(buffer, index) == 'n' &&
	   ReadChar(buffer, index) == 'm' &&
	   ReadChar(buffer, index) == '\0')
	   return true;
	else return false;
}

void PanelAnimFile::Update(float anim_time, const matrix4x4& offset_matrix)
{
	PanelAnim* anim = obs;
	while (anim != NULL)
	{
		anim->Update(anim_time, offset_matrix);  //  No parent, so pass in identity.
		anim = anim->next;
	}
}

PanelAnim *PanelAnimFile::FindObject(char *object_name)
{
	if (obs != NULL)
		return obs->FindObject(object_name);
	else
		return NULL;
}


// Animation Management
PanelAnimManager::~PanelAnimManager()
{
	PanelAnimEvent* tmp = eventlist.next;
	PanelAnimEvent* tmp2;
	while(tmp)
	{
		tmp2 = tmp->next;
		delete tmp;
		tmp = tmp2;
	}
}

//  Add a PanelAnimEvent onto the list of currently playing events.
//  Returns a pointer the the exact event that was added to the list.
PanelAnimEvent *PanelAnimManager::Play(PanelAnimFile* anim, AnimType play_type, int flags)
{
	PanelAnimEvent *new_event = NULL;

	if(flags & ALREADY_PLAYING)
	{
		new_event = Find(*anim);
		flags &= ~ALREADY_PLAYING;
	}

	//  If the event was not already listed, then make room for it.
	if(new_event == NULL)
	{
		new_event = NEW PanelAnimEvent;
		new_event->next = eventlist.next;
		new_event->prev = &eventlist;
		eventlist.next = new_event;
		if(new_event->next)
			new_event->next->prev = new_event;
	}

	new_event->animation = anim;
	PanelAnim* object = new_event->animation->obs;
	while(object)
	{
		object->quad->StartAnim(true);
		object = object->next;
	}
	new_event->current_time = 0;  //  Start and endtime will be set the first time an anim is drawn.
	new_event->start_offset = 0;  //  There are no offsets when we play the whole animation.
	new_event->total_time = new_event->animation->totalseconds;
	new_event->offsets = identity_matrix;

	new_event->flags = flags;

	if(play_type == PING_PONG)
	{
		new_event->type = PLAY;            //  Just start out playing forward.
		new_event->flags |= PING_PONGING;
	}
	else
	{
		new_event->type = play_type;
		new_event->flags &= ~PING_PONGING;
	}

	return new_event;

}


//  Play an animation from a certain start time (within the anim) to a certain end time.
PanelAnimEvent *PanelAnimManager::PlayPart(PanelAnimFile &anim,float start_time, float end_time,
                                AnimType play_type, int flags)
{
	assert(start_time <= anim.totalseconds && end_time <= anim.totalseconds);

	PanelAnimEvent *new_event = Play(&anim, play_type, flags);
	new_event->start_offset = start_time;
	new_event->total_time = end_time - start_time;

	return new_event;
}


//  Just start an anim holding at a particular time in the animation.
PanelAnimEvent *PanelAnimManager::HoldAtTime(PanelAnimFile &anim,float hold_time, int flags)
{
	assert(hold_time <= anim.totalseconds);
	return PlayPart(anim, hold_time, hold_time, PLAY, HOLD_AFTER_PLAYING | flags);
}


//  Remove a currently playing animation from the list.
void PanelAnimManager::Stop(PanelAnimFile &anim)
{
	PanelAnimEvent *dead_event = Find(anim);
	while(dead_event != NULL)
	{
		dead_event->prev->next = dead_event->next;   //  There is always a prev, the head at least.
		if(dead_event->next)
			dead_event->next->prev = dead_event->prev;
		delete dead_event;
		dead_event = Find(anim);
	}
}


//  Return whether or not an animation is in the list AND not holding.
bool PanelAnimManager::IsPlaying(PanelAnimFile &anim)
{
	PanelAnimEvent *found_event = Find(anim);
	if(found_event != NULL)
	{
		if(found_event->current_time < found_event->total_time) //  Is it currently playing?
			return true;
		else if ((found_event->type == PLAY && found_event->flags & PING_PONGING) || //  Will it restart playing soon?
			found_event->flags & LOOP)
			return true;
		else
			return false;
	}
	else
		return false;
}

//  Look through the current animation events and see if anim is already in the list.
//  If it is, then return a pointer to that event.
PanelAnimEvent *PanelAnimManager::Find(PanelAnimFile &anim)
{
	PanelAnimEvent *current_event = eventlist.next;

	while (current_event)
	{
		if(current_event->animation == &anim)
			return current_event;

		current_event = current_event->next;
	}

	//  We didn't find it if we got this far.
	return NULL;

}


//  Play all the animations currently on the list.
void PanelAnimManager::UpdateAnims(time_value_t delta_time)
{
	//  Play the animation events.
	PanelAnimEvent *ce = eventlist.next;

	if (delta_time > .0833333f)   //  Don't let more than a 12th of a second pass.
		delta_time = .083333f;

	while (ce)
	{
		// Only update the time if the object is not currently holding.
		if (!(ce->flags & HOLD_AFTER_PLAYING && ce->current_time >= ce->total_time && !(ce->flags & PING_PONGING && ce->type == PLAY) && !(ce->flags & LOOP)))
				ce->current_time += delta_time;

		if (ce->current_time > ce->total_time &&
			ce->flags & PING_PONGING &&   //  Check for reversing a ping-pong
			ce->type == PLAY)
		{
			ce->current_time -= ce->total_time;
			ce->type = PLAY_BACKWARD;
		}

		if (ce->current_time > ce->total_time && ce->flags & LOOP) //Check for resetting a loop.
		{
			ce->current_time -= ce->total_time;
			if (ce->flags & PING_PONGING)
				ce->type = PLAY;  //  In case this was the end of a ping-pong, start over.
		}


		if (ce->current_time <= ce->total_time)  //  Just play it.
		{
			if (ce->type == PLAY_BACKWARD)
				ce->animation->Update(ce->total_time - ce->current_time + ce->start_offset,
                                           ce->offsets);
			else
				ce->animation->Update(ce->current_time + ce->start_offset, ce->offsets);

			ce = ce->next;
		}
		else if (!(ce->flags & HOLD_AFTER_PLAYING))  //  It's done, remove it from the list.
		{
			PanelAnimEvent *dead_event = ce;
			ce = ce->next;

			dead_event->prev->next = dead_event->next;   //  There is always a prev, the head at least.
			if (dead_event->next)
			dead_event->next->prev = dead_event->prev;
			PanelAnim* object = dead_event->animation->obs;
			while(object)
			{
				object->quad->TurnOn(false);
				object->quad->StartAnim(false);
				object = object->next;
			}
			delete dead_event;
		}
		else  //  No animation, just draw the last frame.
		{
			if (ce->type == PLAY_BACKWARD)
				ce->animation->Update(ce->start_offset, ce->offsets);
			else
//				ce->animation->Update(ce->start_offset + ce->total_time, ce->offsets);
			{
				// code to delete the holding event
				PanelAnimEvent *dead_event = ce;
				ce = ce->next;
				dead_event->prev->next = dead_event->next;   //  There is always a prev, the head at least.
				if (dead_event->next)
				dead_event->next->prev = dead_event->prev;
				PanelAnim* object = dead_event->animation->obs;
				while(object)
				{
					object->quad->StartAnim(false);
					object = object->next;
				}
				delete dead_event;

			}
		}

	}  //  end while ce

}  //  End UpdateAnims()



