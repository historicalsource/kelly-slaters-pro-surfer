
#ifndef INCLUDED_CHALLENGE_ICON_H
#define INCLUDED_CHALLENGE_ICON_H

#include "global.h"
#include "challenge.h"
#include "trickregion.h"

class kellyslater_controller;

// IconChallenge class: implementation and state information for a beach's icon challenge.
// This is the challenge where 3D "icons" hover near the wave.
class IconChallenge : public Challenge
{
public:
	enum { MAX_SEQUENCE_SIZE = 10 };
	enum { MAX_ARRANGEMENT_SIZE = 10 };

private:
	static const float TIME_SEQUENCE_ACTIVE;	// amount of time player has to complete a sequence
	static const float TIME_BETWEEN_SEQS;		// amount of time from when a sequence ends to when the next one starts

public:
	// Icon: 3D entity on the wave.
	class Icon
	{
	private:
		static float PULSE_MIN;						// minimum color intensity
		static float PULSE_MAX;						// maximum color intensity
		static float PULSE_SPEED;					// number of seconds it takes to go from min to max

	private:
		entity *	parentEnt;
		entity *	fgChildEnt;
		entity *	bgChildEnt;
		vector3d	position;
		color		pulseColor;
		int			pulseDir;

	private:
		entity * LoadEntity(const stringx name) const;

	public:
		// Creators.
		Icon();
		~Icon();

		// Modifiers.
		void Init(const stringx name, const int textureIdx, const vector3d pos, const float scale);
		void Update(const float dt);
		void Spawn(void);
		void Despawn(void);

		// Accessors.
		bool IsSpawned(void) const;
		entity * GetEntity(void) { return parentEnt; }
		vector3d GetPosition(void) const { return position; }
	};

	// Task: Requirements to make the associated icon go away.
	class Task
	{
	public:
		static const float REQUIRED_DIST2;	// required distance to player squared

	public:
		enum TYPE
		{
			TYPE_AIR_FLIP,
			TYPE_AIR_GRAB,
			TYPE_AIR_SPIN,
			TYPE_AIR_POINTS_1000,
			TYPE_AIR_POINTS_2000,
			TYPE_AIR_POINTS_5000,
			TYPE_AIR_SPECIAL,

			TYPE_FACE_SPIN,

			TYPE_TUBE_TIME_10,
			TYPE_TUBE_TIME_15,
			TYPE_TUBE_TIME_30,
			TYPE_TUBE_TRICK,

			TYPE_NUM,
			TYPE_NONE,
		};

	private:
		Icon *	icon;
		bool	completed;
		TYPE	type;
		bool	watchChain;

	private:
		void CheckForCompletion(kellyslater_controller * ksctrl);

	public:
		// Creators.
		Task();

		// Modifiers.
		void Reset(void);
		void Init(Icon * i, const TYPE t);
		void Update(kellyslater_controller * ksctrl, const float dt);
		void Spawn(void);
		void Despawn(void);
		void OnEvent(const EVENT event, kellyslater_controller * ksctrl, const int param2 = 0);

		// Accessors.
		bool IsCompleted(void) const { return completed; }
		TYPE GetType(void) const { return type; }
		vector3d GetPosition(void) const { return icon->GetPosition(); }
	};

	// Sequence: a group of icons that must be completed together.
	class Sequence
	{
	private:
		int		numTasks;
		Task	tasks[MAX_SEQUENCE_SIZE];
		bool	completed;

	public:
		// Creators.
		Sequence();

		// Modifiers.
		void Reset(void);
		void Update(kellyslater_controller * ksctrl, const float dt);
		void PushTask(Icon * i, const Task::TYPE t);
		void Spawn(void);
		void Despawn(void);
		void OnEvent(const EVENT event, kellyslater_controller * ksctrl, const int param2 = 0);

		// Accessors.
		bool IsCompleted(void) const { return completed; }
		int GetNumTasks(void) const { return numTasks; }
		const Task * GetTask(const int i) const { if (i >= 0 && i < numTasks) return &tasks[i]; else return NULL; }
	};

	// Arrangement: a list of sequences of icons that spans the entire level.
	class Arrangement
	{
	private:
		int			numSequences;
		int			currSequenceIdx;
		Sequence	sequences[MAX_ARRANGEMENT_SIZE];
		bool		completed;

	public:
		// Creators.
		Arrangement();

		// Modifiers.
		void Init(Icon * icons);
		void Reset(Icon * icons);
		void Update(kellyslater_controller * ksctrl, const float dt);
		void Spawn(void);
		void Despawn(void);
		void OnEvent(const EVENT event, kellyslater_controller * ksctrl, const int param2 = 0);

		// Accessors.
		bool IsCompleted(void) const { return completed; }
		const Sequence * GetCurrentSequence(void) const;
	};

private:
	kellyslater_controller *	ksctrl;
	int							numIcons;					// icon array size
	Icon *						icons;						// icon array
	Arrangement					arrangement;

public:
	// Creators.
	IconChallenge();
	~IconChallenge();

	// Modifiers.
	void Init(kellyslater_controller * ks);
	void Retry(void);
	void Update(const float dt);
	void OnEvent(const EVENT event, const int param1 = 0, const int param2 = 0);

	// Accessors.
	bool IsCompleted(void) const { return arrangement.IsCompleted(); }
	const Sequence * GetCurrentSequence(void) const { return arrangement.GetCurrentSequence(); }
};

#endif INCLUDED_CHALLENGE_ICON_H
