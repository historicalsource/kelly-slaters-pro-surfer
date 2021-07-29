#ifndef BLUR_H
#define BLUR_H

void BLUR_Init(void);
void BLUR_Draw(void);
void BLUR_TurnOn(void);
void BLUR_TurnOff(void);

// Utilities.
u_int RoundUpToPowerOf2(u_int a);

#endif