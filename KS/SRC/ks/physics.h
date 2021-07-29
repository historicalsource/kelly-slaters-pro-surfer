#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "po.h"
#include "ode.h"
#include "algebra.h"

	// copied from the other physics.h
const rational_t DENSITY_OF_WATER = 1000.0f;


class PhysicsObjectClass;
typedef class PhysicsObjectClass *PhysicsObjectClassPtr;

class SimpleForce
{
public:
	int32 Init(vector3d &direction)
	{
		dir = direction;
		dir.normalize();

		s = 0.0f;
		return 1;
	}

	void SetForceScalar(float scalar) { s = scalar;}

	vector3d dir;
	float s;
};

class PhysicsObjectClass
{
public:
  PhysicsObjectClass();
  ~PhysicsObjectClass();

	void Update(float dt);

	vector3d* GetPosition() { return &pos; }
	vector3d* GetForce() { return &force; }

	void GetPosition(vector3d &v) { v = pos; }
	vector3d* GetForce(vector3d &v) { v = force; return &force; }

	float GetInverseMass() { return inverseMass; }
	float GetMass() { return mass; }

	void SetMass(float m) { mass = m; inverseMass = 1.0f/m; }
	void SetPosition(vector3d &v) { pos = v; }
	void SetVelocity(vector3d &v) { velo = v; linMom = mass*velo; }

	void GetState (float *s);
	void SetState (float *s);

	static void DerivFunc (PhysicsObjectClassPtr phyobj, float rTime, float* rInStateArray, float* rOutStateArray);

  float mass;
	float inverseMass;
	vector3d force;
	vector3d velo;
	vector3d pos;

  po my_po;

	// linear //
	vector3d	linMom;

private:
	RungeKuttaClass ode;
};

#endif // __PHYSICS_H__ //
