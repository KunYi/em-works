// update.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <winioctl.h>
#include "pkfuncs.h"

BOOL FileIsExist( LPTSTR lpFileName )
{
	HANDLE hFile;

	hFile = CreateFile( lpFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile==INVALID_HANDLE_VALUE )
	{
		return FALSE;
	}
	CloseHandle( hFile );

	return TRUE;
}

BOOL UpdateFiles( LPCTSTR lpSourcePathName, LPCTSTR lpDestinationPathName )
{
	HANDLE						hFindFile;
	BOOL							bResult;
	TCHAR						PathFileName[MAX_PATH], OldFileName[MAX_PATH], NewFileName[MAX_PATH];
	char							FileName[MAX_PATH];
	WIN32_FIND_DATA		FindFileData;
	int								i1;

	_stprintf( PathFileName, TEXT( "%s\\*.*" ), lpSourcePathName );

	hFindFile = FindFirstFile( PathFileName, &FindFileData );

	if( hFindFile == INVALID_HANDLE_VALUE )
		return FALSE;

	for( ; ; )
	{
		if( wcscmp( FindFileData.cFileName, TEXT( "update.flg" ) )!=0 )
		{
			i1 = wcslen( FindFileData.cFileName );
			wcstombs( FileName, FindFileData.cFileName, i1 );
			FileName[i1] = 0;
			printf( "%s\r\n", FileName );

			_stprintf( NewFileName, TEXT( "%s\\%s" ), lpDestinationPathName, FindFileData.cFileName );
			_stprintf( OldFileName, TEXT( "%s\\%s" ), lpSourcePathName, FindFileData.cFileName );
			bResult = CopyFile( OldFileName, NewFileName, FALSE );
			//bResult = DeleteFile( OldFileName );
		}

		bResult = FindNextFile( hFindFile, &FindFileData );
		if( bResult == FALSE )
			break;

	}

	FindClose( hFindFile );

	return TRUE;
}


void  SysReboot( )
{
	KernelIoControl( IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL );	
}

BOOL GetDiskNameAndDestPathName( LPCTSTR lpStr, LPCTSTR lpDiskName, LPCTSTR lpDestPathName )
{
	int				i1;
	TCHAR		*ptstr1=NULL, *ptstr2=NULL;

	if( wcslen( lpStr ) <= 0 )
		return FALSE;

	// step1: search for DiskName
	ptstr1 = wcsstr( (wchar_t*)lpStr, _T("\\") );
	if( ptstr1==NULL )
		return FALSE;
	ptstr1++;
	ptstr2 = wcsstr( (wchar_t*)ptstr1, _T("\\") );
	if( ptstr2==NULL )
		return FALSE;
	i1 = ptstr2 - ptstr1;
	wcsncpy( (wchar_t*)lpDiskName, ptstr1, i1 );
	ptstr2++;

	// step2: for lpDestPathName
	for( ; ; )
	{
		ptstr1 = wcsstr( ptstr2, _T("\\") );
		if( ptstr1==NULL )
			break;
		ptstr1++;
		ptstr2 = ptstr1;
	}
	i1 = ptstr2 - lpStr - 1;
	wcsncpy( (wchar_t*)lpDestPathName, lpStr, i1 );

	return TRUE;
}


int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	BOOL				b, bResult;
	TCHAR				DiskName[MAX_PATH], SourcePathName[MAX_PATH], DestPathName[MAX_PATH];
	TCHAR				UpdateFlagFileName[MAX_PATH];
	TCHAR				EXE_NAME[MAX_PATH], EXE_PAR[MAX_PATH];
	PROCESS_INFORMATION procInfo;

	bResult = GetDiskNameAndDestPathName( lpCmdLine, DiskName, DestPathName );
	if( bResult==FALSE )
	{
		_tcscpy( DiskName, TEXT("NandFlash" ) );
		_tcscpy( DestPathName, TEXT( "\\NandFlash") );
	}
	_stprintf( SourcePathName, TEXT( "\\%s\\FTPUpdate" ), DiskName );

	_stprintf( UpdateFlagFileName, TEXT( "%s\\update.flg" ), SourcePathName );

	// step1: update program files
	if( FileIsExist( UpdateFlagFileName ) )
	{
		//update program files: copy files from NandFlash FTP directory
		RETAILMSG( 1, (TEXT("\r\n update program files" ) ) );
		bResult = UpdateFiles( SourcePathName, DestPathName );
		if( bResult==TRUE )
		{
			//delete update flag's file
			bResult = DeleteFile( UpdateFlagFileName );
		}
	}

	// step2: start application program exe
	if( wcslen( lpCmdLine ) > 0 )
	{
		_stscanf( lpCmdLine, _T("%s %s"), EXE_NAME, EXE_PAR );

		b = CreateProcess
		( 
			EXE_NAME,				  // LPCWSTR lpszImageName, 
			EXE_PAR,                  // LPCWSTR lpszCmdLine, 
			NULL,                     // LPSECURITY_ATTRIBUTES lpsaProcess, 
			NULL,                     // LPSECURITY_ATTRIBUTES lpsaThread, 
			FALSE,                    // BOOL fInheritHandles, 
			0,						  // DWORD fdwCreate, 
			NULL,                     // LPVOID lpvEnvironment, 
			NULL,                     // LPWSTR lpszCurDir, 
			NULL,                     // LPSTARTUPINFOW lpsiStartInfo, 
			&procInfo                 // LPPROCESS_INFORMATION lppiProcInfo
		); 

		if( b )
		{
			CloseHandle(procInfo.hProcess);
			CloseHandle(procInfo.hThread);		
		}
	}


	// step3: start check file thread
	for( ; ; )
	{
		Sleep( 5000 );				// about 5 seconds to check FTP File Flag
		if( FileIsExist( UpdateFlagFileName ) )
		{
			// need reboot system to update program files
			SysReboot( );
		}
	}
	return 0;
}


