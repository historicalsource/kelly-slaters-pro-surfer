//-----------------------------------------------------------------------------
// File: XbStorageDevice.h
//
// Desc: Hard disk and memory unit devices. Provides save and load game
//       functionality.
//
// Hist: 01.30.01 - New for March XDK release
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef XBSTORAGE_DEVICE_H
#define XBSTORAGE_DEVICE_H

#include <xtl.h>




//-----------------------------------------------------------------------------
// Name: class CXBStorageDevice
// Desc: Xbox storage device (hard drive, MU)
//-----------------------------------------------------------------------------
class CXBStorageDevice
{

    mutable HANDLE m_hSaveGame;         // For iterating through saved games
    CHAR           m_strRootPath[4];    // "@:\" where @ is the logical drive
                                        // letter of the storage device

public:

    explicit CXBStorageDevice( CHAR chDriveLetter = 0 );
    CXBStorageDevice( const CXBStorageDevice& );
    CXBStorageDevice& operator=( const CXBStorageDevice& );
    virtual ~CXBStorageDevice();

    static CXBStorageDevice GetUserRegion();
    static CXBStorageDevice GetTitleRegion();
    static CXBStorageDevice GetUtilityRegion( BOOL fFormatClean );
    static DWORD GetBlockSize();

    CHAR  GetDrive() const;
    VOID  SetDrive( CHAR chDriveLetter );
    BOOL  IsValid() const;
    BOOL  IsTitleRegion() const;
    BOOL  IsUtilityRegion() const;
    BOOL  GetSize( ULONGLONG& qwTotalBlocks, ULONGLONG& qwUsedBlocks, 
                   ULONGLONG& qwFreeBlocks ) const;
    DWORD GetSavedGameCount() const;
    DWORD GetSectorSize() const;
    DWORD GetClusterSize() const;
    DWORD GetSaveGameOverhead() const;
    DWORD GetFileBytes( DWORD ) const;
    BOOL  FindFirstSaveGame( XGAME_FIND_DATA& ) const;
    BOOL  FindNextSaveGame( XGAME_FIND_DATA& ) const;

private:

    VOID Cleanup() const;

};

#endif // XBSTORAGE_DEVICE_H
