#include "global.h"
#include "physics.h"
#include <stdlib.h>

PhysicsObjectClass::PhysicsObjectClass ()
{
  force = vector3d (0, 0, 0);
  velo = vector3d (0, 0, 0);
  pos = vector3d (0, 0, 0);
  my_po = po_identity_matrix;

  mass = 1.0;
  inverseMass = 1.0;

  pos = vector3d (0,0,0);
  linMom = vector3d (0,0,0);

  // init ode
	this->ode.Init(18, 0.1f, PhysicsObjectClass::DerivFunc);
}

PhysicsObjectClass::~PhysicsObjectClass ()
{
  ode.Terminate ();
}

void PhysicsObjectClass::Update (float dt)
{
  // integrate
	float timeout = 0;
	float inState[18];
	float outState[18];
		// GC was getting a nan in here 
	for (int i=0; i<18; i++ ) inState[i]=0.0f;

  this->GetState (inState);
	this->ode.SetStepSize (dt);
	this->ode.Update (this, dt, inState, &timeout, outState);
	this->SetState (outState);
}

void PhysicsObjectClass::GetState(float *s)
{
	// get the position //
	*(s++) = this->pos.x; 
	*(s++) = this->pos.y;
	*(s++) = this->pos.z;

  // linear momentum //
	*(s++) = this->linMom.x; 
	*(s++) = this->linMom.y;
	*(s++) = this->linMom.z;
}

void PhysicsObjectClass::SetState(float *s)
{
	// set the position  x(t)
	this->pos.x  = *(s++);
	this->pos.y  = *(s++);
	this->pos.z  = *(s++);

	// set linear momentum  P(t)
  this->linMom.x = *(s++);
	this->linMom.y = *(s++);
	this->linMom.z = *(s++);

	// --------------------------- //
	// Compute auxiliary variables //
	// --------------------------- //

  // compute velocity v(t) = P(t)/mass
  this->velo = vector3d (this->linMom.x * this->inverseMass, 
  this->linMom.y * this->inverseMass, 
  this->linMom.z * this->inverseMass);
}

void PhysicsObjectClass::DerivFunc(PhysicsObjectClassPtr phyobj, float rTime, float* rInStateArray, float* rOutStateArray)
{
	phyobj->SetState(rInStateArray);

   // copy x'(t) = v(t)
   *(rOutStateArray++) = phyobj->velo.x;
   *(rOutStateArray++) = phyobj->velo.y;
   *(rOutStateArray++) = phyobj->velo.z;

	// (d/dt)P(t) = F(t)
	*(rOutStateArray++) = phyobj->force.x;
	*(rOutStateArray++) = phyobj->force.y;
	*(rOutStateArray++) = phyobj->force.z;
}
