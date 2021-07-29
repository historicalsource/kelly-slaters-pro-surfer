#ifndef __ODE_H__
#define __ODE_H__

class PhysicsObjectClass; // forward referencing

typedef class OdeSolverClass OdeSolverClass;
typedef class OdeSolverClass *OdeSolverClassPtr;

typedef class RungeKuttaClass RungeKuttaClass;
typedef class RungeKuttaClass *RungeKuttaClassPtr;

// for dx/dt = F(t,x)
typedef void (*OdeDerivFunc)(PhysicsObjectClass* phyobj, float rTime,
   float* rInStateArray, float* rOutStateArray);


// pure virtual, simply an ODE interface 	//
class OdeSolverClass
{
public:
	OdeSolverClass() {
		dim = 0;
		stepSize = 0.0f;
		fnDerivFunc = NULL;
	}

	virtual ~OdeSolverClass() { };

	virtual int32 Init(int d, float step, OdeDerivFunc dfn) = 0;
	virtual int32 Terminate() = 0;
	virtual int32 Update(PhysicsObjectClass* phyobj, float timein, float *inArray, float *timeout, float *outArray) = 0;
	virtual void SetStepSize(float dt) { stepSize = dt;}

protected:
	int32			dim;
	float			stepSize;
	OdeDerivFunc	fnDerivFunc; // derivative function dy/dt
};

class RungeKuttaClass : public OdeSolverClass
{

public:
	RungeKuttaClass() {
		temp1 =
		temp2 =
		temp3 = 
		temp4 = 
		xtemp = NULL;
	}

	virtual ~RungeKuttaClass() {
		this->Terminate();
	}
	
	int32 Init(int d, float step, OdeDerivFunc dfn);
	int32 Terminate();
	int32 Update(PhysicsObjectClass* phyobj, float timein, float *inArray, float *timeout, float *outArray);

	virtual void SetStepSize(float dt) 
	{ 
		this->stepSize = dt;
		this->step2 = dt/2.0f;
		this->step6 = dt/6.0f;
	}

private:	
   float step2, step6;
   float* temp1;
   float* temp2;
   float* temp3;
   float* temp4;
   float* xtemp;

};

#endif // __ODE_H__ //
