/*
Copyright (c) 2013-2015, Ralf Willenbacher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include "eng.h"

#ifdef WIN32
#include <windows.h>
#include <d3d9.h>

#include "win32_dsurface.h"
#endif

#define DISPLAY_FORMAT_UNKNOWN 0
#define DISPLAY_FORMAT_RGB32   1

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


class CWin32Display
{
public:
	CWin32Display();
	~CWin32Display();

	// Display Interface Impl.
	Bool Init( );
	Bool ReInit( );
	Bool DeInit( );

	Bool GetNumResolutions( Int32 *piNumResolutions );
	Bool QueryResolution( Int32 iIdx, Int32 *piWidth, Int32 *piHeight, Int32 *piBpp, Int32 *piRefreshRate );
	Bool SetResolution( Int32 iIdx, Int32 iWidth, Int32 iHeight );
	Bool SetFullscreen( Bool bFullscreen );

	Bool BeginRenderScreen( );
	Bool GetFrameBuffer( void **pSurface, UInt32 *piWidth, UInt32 *piHeight, UInt32 *piStride, UInt32 *piBpp );
	Bool FinishRenderScreen( );

	// Loop Iteration interface for message loop
	Bool OnLoopIteration();

	UInt32 m_uiKeys;
private:

	WCHAR *GetWindowClassName();
	WCHAR *GetWindowName();

	IDirect3D9 *m_pDirect3D9;
	IDirect3DDevice9 *m_pDirect3DDevice;

	D3DCAPS9 m_sCaps;
	D3DPRESENT_PARAMETERS m_sD3DParameters; 

	UInt32 m_uiSelectedDevice;
	D3DFORMAT m_eQueriedDisplayModeFormat;
	Int32 m_uiSelectedDisplayMode;

	Bool   m_bFullscreen;
	Bool   m_bCanInit;
	Bool   m_bIsInitialized;
	

	Int32 m_iCurrentWidth;
	Int32 m_iCurrentHeight;
	Int32 m_iCurrentPitch;
	Int32 m_iCurrentFormat;

	Bool   m_bFrameLocked;

	IDirect3DSurface9 *m_pD3DSurface;
	void *m_pFrameBuffer;


	HWND m_hWnd;

};



CWin32Display::CWin32Display()
{
	UInt32 uiIdx, uiNumAdapters;

	// reset members
	m_bCanInit = FALSE;
	m_bIsInitialized = FALSE;
	m_pDirect3D9 = 0;
	m_pDirect3DDevice = 0;
	memset( &m_sCaps, 0, sizeof( m_sCaps ) );
	memset( &m_sD3DParameters, 0, sizeof( m_sD3DParameters ) );
	m_uiSelectedDevice = 0;
	m_uiSelectedDisplayMode = 0;
	m_eQueriedDisplayModeFormat = D3DFMT_P8;
	m_bFrameLocked = FALSE;

	if( NULL == ( m_pDirect3D9 = Direct3DCreate9( D3D_SDK_VERSION ) ) )
	{
		m_bCanInit = FALSE;
	}

	m_bCanInit = TRUE;
	uiNumAdapters = m_pDirect3D9->GetAdapterCount();

	for( uiIdx = 0; uiIdx < uiNumAdapters; uiIdx++ )
	{
		HRESULT hCall;

//uiIdx or D3DADAPTER_DEFAULT
		hCall = m_pDirect3D9->CheckDeviceType( uiIdx, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, TRUE );
		if( SUCCEEDED( hCall ) )
		{
			m_uiSelectedDevice = uiIdx;
		}
		else
		{
			m_bCanInit = FALSE;
		}

		hCall = m_pDirect3D9->GetDeviceCaps( uiIdx, D3DDEVTYPE_HAL, &m_sCaps );
		if( SUCCEEDED( hCall ) )
		{
			m_sCaps.PixelShaderVersion;
			m_sCaps.MaxPShaderInstructionsExecuted;
			m_sCaps.MaxPixelShader30InstructionSlots;
		}
		else
		{
			m_bCanInit = FALSE;
		}
	}

	// register window class
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL, GetWindowClassName(), NULL };
	RegisterClassEx( &wc );


	m_uiKeys = 0;
}

CWin32Display::~CWin32Display()
{
	if( m_bIsInitialized )
	{
		DeInit();
	}
	m_pDirect3D9->Release();
	m_pDirect3D9 = 0;

	// unregister window class
	UnregisterClass( GetWindowClassName(), GetModuleHandle( NULL ) );
}

WCHAR *CWin32Display::GetWindowClassName()
{
	return TEXT( "CWin32Display" );
}

WCHAR *CWin32Display::GetWindowName()
{
	return TEXT( "CWin32DisplayImpl" );
}


Bool CWin32Display::Init()
{
	Int32 iWidth, iHeight, iFormat, iRefreshRate;
	HRESULT hr;

	if( m_bIsInitialized )
	{
		return FALSE;
	}

	if( m_uiSelectedDisplayMode >= 0 )
	{
		if ( QueryResolution( m_uiSelectedDisplayMode, &iWidth, &iHeight, &iFormat, &iRefreshRate ) )
		{
			m_iCurrentWidth = iWidth;
			m_iCurrentHeight = iHeight;
			m_iCurrentFormat = iFormat;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{

	}
	m_iCurrentPitch = 0;

	RECT rect;
	rect.top = 40;
	rect.bottom = rect.top + 100;
	rect.left = 20;
	rect.right = rect.left + 160;

	AdjustWindowRect( &rect, WS_OVERLAPPEDWINDOW, FALSE );

	m_hWnd = CreateWindow( GetWindowClassName(), GetWindowName(), 
		WS_OVERLAPPEDWINDOW, rect.top, rect.left, rect.right - rect.left, rect.bottom - rect.top, GetDesktopWindow(), NULL, GetModuleHandle( NULL ), NULL );


	if( m_hWnd == NULL )
	{
		return FALSE;
	}

	ZeroMemory( &m_sD3DParameters, sizeof( m_sD3DParameters ) );
	m_sD3DParameters.Windowed = m_bFullscreen ? FALSE : TRUE;
	m_sD3DParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_sD3DParameters.BackBufferFormat = D3DFMT_A8R8G8B8;
	m_sD3DParameters.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	//m_sD3DParameters.lo


	if( FAILED( hr = m_pDirect3D9->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
                D3DCREATE_SOFTWARE_VERTEXPROCESSING, // lets hope the driver does the right thing.
                &m_sD3DParameters, &m_pDirect3DDevice ) ) )
	{
		m_bCanInit = FALSE;

		CloseWindow( m_hWnd );
		DestroyWindow( m_hWnd );

		return FALSE;
	}


	ShowWindow(m_hWnd, SW_SHOWDEFAULT); 
	UpdateWindow( m_hWnd ); 

	m_bIsInitialized = TRUE;


	return TRUE;
}


Bool CWin32Display::DeInit()
{
	if( m_bIsInitialized )
	{
		CloseWindow( m_hWnd );
		DestroyWindow( m_hWnd );

		m_pDirect3DDevice->Release();
		m_bIsInitialized = FALSE;
		return TRUE;
	}
	return FALSE;
}

Bool CWin32Display::ReInit()
{
	if( DeInit() )
	{
		return Init();
	}
	return FALSE;
}



Bool CWin32Display::GetNumResolutions(Int32 *piNumResolutions)
{
	Int32 iNumResolutions;

	m_eQueriedDisplayModeFormat = D3DFMT_A8R8G8B8;
	iNumResolutions = (Int32) m_pDirect3D9->GetAdapterModeCount( m_uiSelectedDevice, D3DFMT_A8R8G8B8 );
	if ( iNumResolutions == 0 )
	{
		m_eQueriedDisplayModeFormat = D3DFMT_X8R8G8B8;
		iNumResolutions = (Int32) m_pDirect3D9->GetAdapterModeCount( m_uiSelectedDevice, D3DFMT_X8R8G8B8 );
	}
	*piNumResolutions = iNumResolutions;
	return TRUE;
}

Bool CWin32Display::QueryResolution(Int32 iIdx, Int32 *piWidth, Int32 *piHeight, Int32 *piFormat, Int32 *piRefreshRate)
{
	D3DDISPLAYMODE sDisplayMode;

	if( m_pDirect3D9->EnumAdapterModes( m_uiSelectedDevice, m_eQueriedDisplayModeFormat, (UINT) iIdx, &sDisplayMode ) == S_OK )
	{
		*piWidth = sDisplayMode.Width;
		*piHeight = sDisplayMode.Height;
		*piFormat = sDisplayMode.Format == m_eQueriedDisplayModeFormat ? DISPLAY_FORMAT_RGB32 : DISPLAY_FORMAT_UNKNOWN;
		*piRefreshRate = sDisplayMode.RefreshRate;
		return TRUE;
	}
	else
	{
		*piWidth = *piHeight = *piFormat = *piRefreshRate = 0;
		return FALSE;
	}
}

Bool CWin32Display::SetResolution( Int32 iIdx, Int32 iWidth, Int32 iHeight )
{
	m_uiSelectedDisplayMode = iIdx;
	m_iCurrentWidth = iWidth;
	m_iCurrentHeight = iHeight;
	m_iCurrentFormat = D3DFMT_A8R8G8B8;

	return TRUE;
}

Bool CWin32Display::SetFullscreen(Bool bFullscreen)
{
	m_bFullscreen = bFullscreen;
	return TRUE;
}

Bool CWin32Display::BeginRenderScreen()
{
	HRESULT hr;
	D3DSURFACE_DESC sSurfaceDesc;
	D3DLOCKED_RECT sRect;

	if ( !m_bIsInitialized )
	{
		return FALSE;
	}

	hr = m_pDirect3DDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pD3DSurface );

	if( hr != S_OK )
	{
		return FALSE;
	}

	m_pD3DSurface->GetDesc( &sSurfaceDesc );

	m_iCurrentWidth = sSurfaceDesc.Width;
	m_iCurrentHeight = sSurfaceDesc.Height;
	m_iCurrentFormat = DISPLAY_FORMAT_RGB32;

	hr = m_pD3DSurface->LockRect( &sRect, NULL, 0 );

	if( hr != S_OK )
	{
		return FALSE;
	}

	m_bFrameLocked = TRUE;

	m_pFrameBuffer = sRect.pBits;
	m_iCurrentPitch = sRect.Pitch;

	//memset( m_pFrameBuffer, 0xff, m_iCurrentHeight * m_iCurrentPitch );

	return TRUE;
}

Bool CWin32Display::GetFrameBuffer(void **pSurface, UInt32 *piWidth, UInt32 *piHeight, UInt32 *piStride, UInt32 *piBpp)
{
	if( !m_bIsInitialized )
	{
		return FALSE;
	}

	*pSurface = m_pFrameBuffer;
	*piWidth  = m_iCurrentWidth;
	*piHeight = m_iCurrentHeight;
	*piStride = m_iCurrentPitch;
	*piBpp = m_iCurrentFormat;

	return TRUE;
}

Bool CWin32Display::FinishRenderScreen()
{
	HRESULT hr;

	if( !m_bIsInitialized )
	{
		return FALSE;
	}

	if( m_bFrameLocked )
	{
		hr = m_pD3DSurface->UnlockRect();
		hr = m_pD3DSurface->Release();
	}

	hr = m_pDirect3DDevice->Present( NULL, NULL, NULL, NULL );

	return TRUE;
}


Bool CWin32Display::OnLoopIteration()
{
	if( m_bIsInitialized )
	{
		MSG msg; 
		while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			switch(msg.message) 
			{ 
				case WM_KEYDOWN: 
					if( msg.wParam == VK_ESCAPE )
						m_uiKeys |= ENG_KEY_ESC;
					else if( msg.wParam == VK_LEFT )
						m_uiKeys |= ENG_KEY_LEFT;
					else if( msg.wParam == VK_UP )
						m_uiKeys |= ENG_KEY_UP;
					else if( msg.wParam == VK_RIGHT )
						m_uiKeys |= ENG_KEY_RIGHT;
					else if( msg.wParam == VK_DOWN )
						m_uiKeys |= ENG_KEY_DOWN;
					else if( msg.wParam == VK_RETURN )
						m_uiKeys |= ENG_KEY_ENTER;
					else if( msg.wParam == 0x30 )
						m_uiKeys |= ENG_KEY_0;
					else if( msg.wParam == 0x31 )
						m_uiKeys |= ENG_KEY_1;
					else if( msg.wParam == 0x32 )
						m_uiKeys |= ENG_KEY_2;
					else if( msg.wParam == 0x33 )
						m_uiKeys |= ENG_KEY_3;
					else if( msg.wParam == 0x36 )
						m_uiKeys |= ENG_KEY_6;
				break;
				case WM_KEYUP: 
					if( msg.wParam == VK_ESCAPE )
						m_uiKeys &= ~ENG_KEY_ESC;
					else if( msg.wParam == VK_LEFT )
						m_uiKeys &= ~ENG_KEY_LEFT;
					else if( msg.wParam == VK_UP )
						m_uiKeys &= ~ENG_KEY_UP;
					else if( msg.wParam == VK_RIGHT )
						m_uiKeys &= ~ENG_KEY_RIGHT;
					else if( msg.wParam == VK_DOWN )
						m_uiKeys &= ~ENG_KEY_DOWN;
					else if( msg.wParam == VK_RETURN )
						m_uiKeys &= ~ENG_KEY_ENTER;
					else if( msg.wParam == 0x30 )
						m_uiKeys &= ~ENG_KEY_0;
					else if( msg.wParam == 0x31 )
						m_uiKeys &= ~ENG_KEY_1;
					else if( msg.wParam == 0x32 )
						m_uiKeys &= ~ENG_KEY_2;
					else if( msg.wParam == 0x33 )
						m_uiKeys &= ~ENG_KEY_3;
					else if( msg.wParam == 0x36 )
						m_uiKeys &= ~ENG_KEY_6;
				break;
	        } 
		    TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		return TRUE;
	}
	return FALSE;
}


// windows main loop
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;

        case WM_PAINT:
//            Render(); we redraw manualy
            ValidateRect( hWnd, NULL );
            return 0;
    }
    return DefWindowProc( hWnd, msg, wParam, lParam );
}



Bool __cdecl sys_win32_init( sys_win32_t *p_win32 )
{
	CWin32Display *p_disp;

	p_win32->p_sys_private = new CWin32Display();

	p_disp = ( CWin32Display * )p_win32->p_sys_private;
	
	p_disp->SetFullscreen( FALSE );
	p_disp->SetResolution( -1, DRAWBUFFER_SCREEN_WIDTH, DRAWBUFFER_SCREEN_HEIGHT );
	return p_disp->Init();
}

Bool __cdecl sys_win32_commit_drawbuffer( sys_win32_t *p_win32, UInt8 *pui8_drawbuffer, Int32 i_offs_x, Int32 i_offs_y, Int32 i_fwidth, Int32 i_fheight )
{
	CWin32Display *p_disp;
	UInt32 rgui_pels[ 4 ] = { 0xffffffff, 0xff555555, 0xffaaaaaa, 0xff000000 };
	UInt32 *pui_fb;
	Int32 i_x, i_y;
	UInt32 i_width, i_height, i_stride, i_bpp;

	p_disp = ( CWin32Display * )p_win32->p_sys_private;

	p_disp->BeginRenderScreen();
	p_disp->GetFrameBuffer( ( void ** )&pui_fb, &i_width, &i_height, &i_stride, &i_bpp );

	i_width = min( i_width, i_fwidth );
	i_height = min( i_height, i_fheight );

	for( i_y = 0; i_y < i_height; i_y++ )
	{
		for( i_x = 0; i_x < i_width; i_x+=4 )
		{
			UInt8 ui8_pel = pui8_drawbuffer[ ( i_y * DRAWBUFFER_SCREEN_WIDTH + i_x ) >> 2 ];
			pui_fb[ ( i_y + i_offs_y ) * ( i_stride >> 2 ) + ( i_x + i_offs_x ) ] = rgui_pels[ ( ( ui8_pel & 0x80 ) >> 6 ) | ( ( ui8_pel & 0x8 ) >> 3 ) ];
			pui_fb[ ( i_y + i_offs_y ) * ( i_stride >> 2 ) + ( i_x + i_offs_x ) + 1 ] = rgui_pels[ ( ( ui8_pel & 0x40 ) >> 5 ) | ( ( ui8_pel & 0x4 ) >> 2 ) ];
			pui_fb[ ( i_y + i_offs_y ) * ( i_stride >> 2 ) + ( i_x + i_offs_x ) + 2 ] = rgui_pels[ ( ( ui8_pel & 0x20 ) >> 4 ) | ( ( ui8_pel & 0x2 ) >> 1 ) ];
			pui_fb[ ( i_y + i_offs_y ) * ( i_stride >> 2 ) + ( i_x + i_offs_x ) + 3 ] = rgui_pels[ ( ( ui8_pel & 0x10 ) >> 3 ) | ( ( ui8_pel & 0x1 ) ) ];
		}
	}

	p_disp->FinishRenderScreen();
	return p_disp->OnLoopIteration();


}

Bool __cdecl sys_win32_deinit( sys_win32_t *p_win32 )
{
	CWin32Display *p_disp;

	p_disp = ( CWin32Display * )p_win32->p_sys_private;
	p_disp->DeInit();
	delete p_disp;

	return TRUE;
}

Bool sys_test_key( sys_win32_t *p_win32, Int32 iKey )
{
	CWin32Display *p_disp;

	p_disp = ( CWin32Display * )p_win32->p_sys_private;
	if( p_disp->m_uiKeys & iKey )
	{
		return TRUE;
	}
	return FALSE;
}
