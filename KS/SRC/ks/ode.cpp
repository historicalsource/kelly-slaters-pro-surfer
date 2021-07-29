#include "global.h"

#include "ode.h"
#include "entity.h"


int32 RungeKuttaClass::Init(int d, float step, OdeDerivFunc dfn)
{
	this->dim = d;
	this->stepSize = step;
	this->fnDerivFunc = dfn;
	
	
	this->step2 = step/2.0f;
	this->step6 = step/6.0f;

	this->temp1 = (float*)malloc(sizeof(float) * d);
	this->temp2 = (float*)malloc(sizeof(float) * d);
	this->temp3 = (float*)malloc(sizeof(float) * d);
	this->temp4 = (float*)malloc(sizeof(float) * d);
	this->xtemp = (float*)malloc(sizeof(float) * d);

	if(!this->temp1 || !this->temp2 || !this->temp3 || !this->temp4 || !this->xtemp)
		return -1;

	return 1;
	
}

int32 RungeKuttaClass::Terminate()
{
	if(this->temp1)
		free(this->temp1);
	this->temp1=NULL;

	if(this->temp2)
		free(this->temp2);
	this->temp2=NULL;

	if(this->temp3)
		free(this->temp3);
	this->temp3=NULL;

	if(this->temp4)
		free(this->temp4);
	this->temp4=NULL;

	if(this->xtemp)
		free(this->xtemp);
	this->xtemp=NULL;

	return 1;
}

int32 RungeKuttaClass::Update(PhysicsObjectClass* phyobj, float timein, float *inArray, float *timeout, float *outArray)
{

	int32 i;
	float timeHalf;

   // first step
   (this->fnDerivFunc)(phyobj, timein, inArray, this->temp1);

   for (i = 0; i < this->dim; i++)
   {
      this->xtemp[i] = inArray[i] + this->step2 * this->temp1[i];
   }


   // second step
   timeHalf = timein + this->step2;
   (this->fnDerivFunc)(phyobj, timeHalf, this->xtemp, this->temp2);

   for (i = 0; i < this->dim; i++)
   {
      this->xtemp[i] = inArray[i] + this->step2 * this->temp2[i];
   }


   // third step
   (this->fnDerivFunc)(phyobj, timeHalf, this->xtemp, this->temp3);
   for (i = 0; i < this->dim; i++)
   {
      this->xtemp[i] = inArray[i] + this->stepSize * this->temp3[i];
   }


   // fourth step
   *timeout = timein + this->stepSize;
   (this->fnDerivFunc)(phyobj, *timeout, this->xtemp, this->temp4);
   for (i = 0; i < this->dim; i++)
   {
      outArray[i] = inArray[i] +
         this->step6 * (this->temp1[i] + 2.0f * (this->temp2[i] + this->temp3[i]) + this->temp4[i]);
   }

   return 1;

}



