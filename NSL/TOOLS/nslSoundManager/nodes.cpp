// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "nodes.h"

// Dispatch interfaces referenced by this interface
#include "node.h"


/////////////////////////////////////////////////////////////////////////////
// CNodes properties

/////////////////////////////////////////////////////////////////////////////
// CNodes operations

short CNodes::GetCount()
{
	short result;
	InvokeHelper(0x1, DISPATCH_PROPERTYGET, VT_I2, (void*)&result, NULL);
	return result;
}

void CNodes::SetCount(short nNewValue)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x1, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CNode CNodes::Add(VARIANT* Relative, VARIANT* Relationship, VARIANT* Key, VARIANT* Text, VARIANT* Image, VARIANT* SelectedImage)
{
	LPDISPATCH pDispatch;
	static BYTE parms[] =
		VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT;
	InvokeHelper(0x2, DISPATCH_METHOD, VT_DISPATCH, (void*)&pDispatch, parms,
		Relative, Relationship, Key, Text, Image, SelectedImage);
	return CNode(pDispatch);
}

void CNodes::Clear()
{
	InvokeHelper(0x3, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

CNode CNodes::GetItem(VARIANT* Index)
{
	LPDISPATCH pDispatch;
	static BYTE parms[] =
		VTS_PVARIANT;
	InvokeHelper(0x4, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, parms,
		Index);
	return CNode(pDispatch);
}

void CNodes::SetItem(VARIANT* Index, LPDISPATCH newValue)
{
	static BYTE parms[] =
		VTS_PVARIANT VTS_DISPATCH;
	InvokeHelper(0x4, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 Index, newValue);
}

void CNodes::Remove(VARIANT* Index)
{
	static BYTE parms[] =
		VTS_PVARIANT;
	InvokeHelper(0x5, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Index);
}

LPDISPATCH CNodes::_NewEnum()
{
	LPDISPATCH result;
	InvokeHelper(0xfffffffc, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, NULL);
	return result;
}
