/*  THIS FILE IS AUTOMATICALLY GENERATED BY EXPORT FROM THE EXCEL FILE:  COMBODATA.XLS
    ALL CHANGES SHOULD BE MADE IN EXCEL AND RE_EXPORTED.  DO NOT HAND EDIT.
    Toby Lael, Treyarch, 5/28/02
*/

#ifndef COMBODATA_H
#define COMBODATA_H

#include "global.h"
#include "trickdata.h"

#define MAX_TRICKS_PER_COMBO  5 
#define COMBO_COUNT  6 

#define NONE 0

enum VARTYPE
{
    TYPE_NONE,
    TYPE_TRICK,
    TYPE_GAP,
    TYPE_REGION,
    TYPE_TRICK_TYPE,
};


struct ComboData
{
    int vars[5];
    int type[5];
    int gap;
};

extern ComboData g_comboDataArray[];

#endif /* #ifndef COMBODATA_H */
