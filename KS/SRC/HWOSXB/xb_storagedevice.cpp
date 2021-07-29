//-----------------------------------------------------------------------------
// File: XbStorageDevice.cpp
//
// Desc: Hard disk and memory unit devices. Provides save and load game
//       functionality.
//
// Hist: 01.30.01 - New for March XDK release
//       04.13.01 - Added GetClusterSize(), GetSaveGameOverhead() and
//                  GetFileBytes() for more accurate calculation of
//                  file sizes.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "xb_storagedevice.h"
#include <cassert>




//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
const CHAR* strROOT_PATH = "#:\\";  // "#:\", where # is the drive letter
const DWORD BLOCK_SIZE = 16384;     // TCR 3-11 Space Display




//-----------------------------------------------------------------------------
// Name: CXBStorageDevice()
// Desc: Construct storage device
//-----------------------------------------------------------------------------
CXBStorageDevice::CXBStorageDevice( CHAR chDriveLetter )
:
    m_hSaveGame( INVALID_HANDLE_VALUE ),
    m_strRootPath()
{
    lstrcpyA( m_strRootPath, strROOT_PATH );
    *m_strRootPath = chDriveLetter;
}




//-----------------------------------------------------------------------------
// Name: CXBStorageDevice()
// Desc: Copy storage device
//-----------------------------------------------------------------------------
CXBStorageDevice::CXBStorageDevice( const CXBStorageDevice& rhs )
:
    m_hSaveGame( INVALID_HANDLE_VALUE ),
    m_strRootPath()
{
    lstrcpyA( m_strRootPath, strROOT_PATH );
    *m_strRootPath = *rhs.m_strRootPath;
}




//-----------------------------------------------------------------------------
// Name: CXBStorageDevice()
// Desc: Copy storage device
//-----------------------------------------------------------------------------
CXBStorageDevice& CXBStorageDevice::operator=( const CXBStorageDevice& rhs )
{
    Cleanup();
    *m_strRootPath = *rhs.m_strRootPath;
    return *this;
}




//-----------------------------------------------------------------------------
// Name: ~CXBStorageDevice()
// Desc: Tear down the storage device
//-----------------------------------------------------------------------------
CXBStorageDevice::~CXBStorageDevice()
{
    Cleanup();
}




//-----------------------------------------------------------------------------
// Name: GetUserRegion()
// Desc: Get the object representing the save/load region of the Xbox hard drive
//-----------------------------------------------------------------------------
CXBStorageDevice CXBStorageDevice::GetUserRegion() // static
{
    return CXBStorageDevice( 'U' );
}




//-----------------------------------------------------------------------------
// Name: GetTitleRegion()
// Desc: Get the object representing the title persistent data region of the
//       Xbox hard drive
//-----------------------------------------------------------------------------
CXBStorageDevice CXBStorageDevice::GetTitleRegion() // static
{
    return CXBStorageDevice( 'T' );
}




//-----------------------------------------------------------------------------
// Name: GetUtilityRegion()
// Desc: Get the object representing the utility region of the Xbox hard drive
//-----------------------------------------------------------------------------
CXBStorageDevice CXBStorageDevice::GetUtilityRegion( BOOL fFormatClean ) // static
{
    XMountUtilityDrive( fFormatClean );
    return CXBStorageDevice( 'Z' );
}




//-----------------------------------------------------------------------------
// Name: GetBlockSize()
// Desc: Get the size of a device block in bytes
//-----------------------------------------------------------------------------
DWORD CXBStorageDevice::GetBlockSize() // static
{
    // TCR 3-11 Space Display
    return BLOCK_SIZE;
}




//-----------------------------------------------------------------------------
// Name: GetDrive()
// Desc: Returns the drive letter of this device. Returns 0 if device
//       identifies a null device.
//-----------------------------------------------------------------------------
CHAR CXBStorageDevice::GetDrive() const
{
    return *m_strRootPath;
}




//-----------------------------------------------------------------------------
// Name: SetDrive()
// Desc: Changes the device to refer to a new drive. Invalidates any save game
//       enumeration.
//-----------------------------------------------------------------------------
VOID CXBStorageDevice::SetDrive( CHAR chDriveLetter )
{
    Cleanup();
    *m_strRootPath = chDriveLetter;
}




//-----------------------------------------------------------------------------
// Name: IsValid()
// Desc: TRUE if object specifies a valid storage device (drive letter != 0)
//-----------------------------------------------------------------------------
BOOL CXBStorageDevice::IsValid() const
{
    return( *m_strRootPath != 0 );
}




//-----------------------------------------------------------------------------
// Name: IsTitleRegion()
// Desc: TRUE if device specifies location of title persistent storage
//-----------------------------------------------------------------------------
BOOL CXBStorageDevice::IsTitleRegion() const
{
    return( *m_strRootPath == 'T' );
}




//-----------------------------------------------------------------------------
// Name: IsUtilityRegion()
// Desc: TRUE if device specifies location of utility/cache region
//-----------------------------------------------------------------------------
BOOL CXBStorageDevice::IsUtilityRegion() const
{
    return( *m_strRootPath == 'Z' );
}




//-----------------------------------------------------------------------------
// Name: GetSize()
// Desc: Get the total, used and free bytes. These values are always cluster
//       based; in other words, they're always evenly divisible by the
//       cluster size of the device.
//-----------------------------------------------------------------------------
BOOL CXBStorageDevice::GetSize( ULONGLONG& qwTotalBytes, ULONGLONG& qwUsedBytes,
                                ULONGLONG& qwFreeBytes ) const
{
    if( !IsValid() )
        return FALSE;

    ULARGE_INTEGER lFreeBytesAvailable;
    ULARGE_INTEGER lTotalNumberOfBytes;
    ULARGE_INTEGER lTotalNumberOfFreeBytes;

    BOOL bSuccess = GetDiskFreeSpaceEx( m_strRootPath,
                                        &lFreeBytesAvailable,
                                        &lTotalNumberOfBytes,
                                        &lTotalNumberOfFreeBytes );

    if( !bSuccess )
        return FALSE;

    qwTotalBytes = lTotalNumberOfBytes.QuadPart;
    qwFreeBytes  = lFreeBytesAvailable.QuadPart;
    qwUsedBytes  = qwTotalBytes - qwFreeBytes;

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: GetSavedGameCount()
// Desc: Returns the number of games currently saved on the device
//-----------------------------------------------------------------------------
DWORD CXBStorageDevice::GetSavedGameCount() const
{
    // Use a local CXBStorageDevice object so we don't affect the state of
    // the save game handle.
    CXBStorageDevice thisDevice( *this );
    XGAME_FIND_DATA SaveGameData;

    // Any saves?
    if( !thisDevice.FindFirstSaveGame( SaveGameData ) )
        return 0;

    // At least one; count up the rest
    DWORD dwCount = 1;
    while( thisDevice.FindNextSaveGame( SaveGameData ) )
        ++dwCount;

    return dwCount;
}




//-----------------------------------------------------------------------------
// Name: GetSectorSize()
// Desc: Returns the sector size of the device in bytes
//-----------------------------------------------------------------------------
DWORD CXBStorageDevice::GetSectorSize() const
{
    return XGetDiskSectorSize( m_strRootPath );
}




//-----------------------------------------------------------------------------
// Name: GetClusterSize()
// Desc: Returns the cluster size of the device in bytes
//-----------------------------------------------------------------------------
DWORD CXBStorageDevice::GetClusterSize() const
{
    return XGetDiskClusterSize( m_strRootPath );
}




//-----------------------------------------------------------------------------
// Name: GetOverhead()
// Desc: Number of bytes used on device by XCreateSaveGame. Includes the
//       size of the save game directory and text meta data.
//-----------------------------------------------------------------------------
DWORD CXBStorageDevice::GetSaveGameOverhead() const
{
    // XCreateSaveGame() uses two clusters, one for the saved game directory,
    // and one for the saved game text meta data file.

    return( GetClusterSize() * 2 );
}




//-----------------------------------------------------------------------------
// Name: GetFileBytes()
// Desc: The total number of bytes a file will consume on the device.
//       Automatically pads to the cluster size of the device.
//-----------------------------------------------------------------------------
DWORD CXBStorageDevice::GetFileBytes( DWORD dwBytes ) const
{
    DWORD dwClusterSize = GetClusterSize();

    // Determine clusters required for this file
    DWORD dwClusters = ( dwBytes + ( dwClusterSize-1 ) ) / dwClusterSize;

    // Return total bytes
    return( dwClusters * dwClusterSize );
}




//-----------------------------------------------------------------------------
// Name: FindFirstSaveGame()
// Desc: Returns information about the first save game on the device. Returns
//       FALSE if no saves on the device.
//-----------------------------------------------------------------------------
BOOL CXBStorageDevice::FindFirstSaveGame( XGAME_FIND_DATA& SaveGameData ) const
{
    Cleanup();
    if( *m_strRootPath == 0 )
        return FALSE;

    m_hSaveGame = XFindFirstSaveGame( m_strRootPath, &SaveGameData );
    return( m_hSaveGame != INVALID_HANDLE_VALUE );
}




//-----------------------------------------------------------------------------
// Name: FindNextSaveGame()
// Desc: Returns information about the next save game on the device. Returns
//       FALSE if no more saves on the device.
//-----------------------------------------------------------------------------
BOOL CXBStorageDevice::FindNextSaveGame( XGAME_FIND_DATA& SaveGameData ) const
{
    return XFindNextSaveGame( m_hSaveGame, &SaveGameData );
}




//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Tear down the storage device
//-----------------------------------------------------------------------------
VOID CXBStorageDevice::Cleanup() const
{
    if( m_hSaveGame != INVALID_HANDLE_VALUE )
    {
        XFindClose( m_hSaveGame );
        m_hSaveGame = INVALID_HANDLE_VALUE;
    }
}
