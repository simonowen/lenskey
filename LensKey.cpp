// LensKey - a Lenslok Emulator
//
//	Copyright (c) 2002	Simon Owen <simon.owen@simcoupe.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "LensKey.h"
#include "resource.h"

HINSTANCE __hinstance;
HWND g_hdlg;
RECT rView;

const int STRIPS = 16;			// 16 vertical strips in the lens viewer
const int STRIP_HEIGHT = 32;	// Sample strips taken are stored as 32-pixels high

const char* const WINDOW_CLASS = "LensKeyClass";


// 16 strip values: 6 for each character, with a strip of padding around each character
// The values indicate the position a strip is taken from, as a percentage of the selection width
// Negative values are to the left of the selection, and positive to the right
// Zero is a special case and will be drawn as background colour to leave a gap
const int aanDecode[][16] = 
{
	{ 0, -81, -31,	13, -62, -41,  22, 0,  0, -22,	39, 58, -12, 29, 70, 0 },	// ACE
	{ 0, -41, -30, -68, -52, -11, -20, 0,  0,  32,	60, 11,  22, 49, 71, 0 },	// Art Studio
	{ 0, -41, -57, -77,  10, -28, -19, 0,  0,  43, -10, 22,  32, 77, 58, 0 },	// Elite
	{ 0, -40, -57, -71,  14, -27, -21, 0,  0,  42, -12, 22,  27, 67, 53, 0 },	// Jewels of Darkness
	{ 0, -27, -39, -71, -6,  -17, -48, 0,  0,  51,	64,  7,  40, 17, 79, 0 },	// Price of Magik
	{ 0, -82, -31, -58, -20, -42,  10, 0,  0, -10,	32, 65,  20, 44, 80, 0 },	// Tomahawk
	{ 0, -20, -41, -69, -53,   6, -29, 0,  0,  -9,	64, 20,  46, 33, 81, 0 }	// TT Racer
};

// Title descriptions for the combo box - must match the order in the table above
const char* aszGames[] =
{
	"ACE", "Art Studio", "Elite", "Jewels of Darkness", "Price of Magik", "Tomahawk", "TT Racer",
	NULL
};


// Dialog procedure for the main viewer dialog
BOOL CALLBACK DlgProc (HWND hdlg_, UINT uMsg_, WPARAM wParam_, LPARAM lParam_)
{
	static int nTimer, nDecode = 0;

	switch (uMsg_)
	{
		case WM_INITDIALOG:
		{
			// Set the viewer window as top-most, so it's always visible
			SetWindowPos(g_hdlg = hdlg_, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE);

			// Start a timer to refresh the view window 10 times a second
			nTimer = SetTimer(hdlg_, 1, 100, NULL);

			// Add each title string to the combo box
			HWND hwndCombo = GetDlgItem(hdlg_, IDC_TITLE);
			for (const char** ppcsz = aszGames ; *ppcsz ; ppcsz++)
				SendMessage(hwndCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(*ppcsz));
			SendMessage(hwndCombo, CB_SETCURSEL, 0, 0L);

			return TRUE;
		}

		case WM_COMMAND:
			if (wParam_ == IDCANCEL)
			{
				// Stop the timer and close the dialog
				KillTimer(hdlg_, nTimer);
				EndDialog(hdlg_, 0);
			}
			break;

		case WM_TIMER:
			// Invalidate the dialog to force a redraw
			InvalidateRect(hdlg_, NULL, FALSE);
			return 0;

		case WM_LBUTTONDOWN:
		{
			// Fetch the size of the work area, which excludes the taskbar
			RECT rect;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

			// Create the transparent selection window, filling the entire work area
			HWND hwnd = CreateWindowEx(WS_EX_TRANSPARENT|WS_EX_TOPMOST, WINDOW_CLASS, "", WS_POPUP|WS_VISIBLE,
										rect.left, rect.top, rect.right, rect.bottom, g_hdlg, NULL, __hinstance, NULL);
			break;
		}

		case WM_PAINT:
		{
			// Get the main screen DC
			HDC hdcS = GetDC(NULL);

			// Create a compatible DC and a bitmap to go in it
			HDC hdcMem = CreateCompatibleDC(hdcS);
			HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem, CreateCompatibleBitmap(hdcS, STRIPS, STRIP_HEIGHT));
			BitBlt(hdcMem, 0, 0, STRIPS, STRIP_HEIGHT, NULL, 0, 0, WHITENESS);

			// The decode table used depends on the title combo box selection
			const int* panDecode = aanDecode[SendDlgItemMessage(hdlg_, IDC_TITLE, CB_GETCURSEL, 0, 0L)];

			// Have we got anything to draw?
			if (rView.right > 1)
			{
				// Decode the image by taking vertical strips at the appropriate positions
				for (int i = 0 ; i < STRIPS ; i++)
				{
					// Work out the pixel offset, using an offset off the end for zero (spacing gap)
					int nOffset = MulDiv(rView.right, panDecode[i] ? panDecode[i] : 101, 100);

					// Copy a slice to our memory bitmap
					StretchBlt(hdcMem, i, 0, 1, STRIP_HEIGHT, hdcS, rView.left+nOffset, rView.top+1, 1, rView.bottom-2, SRCCOPY);
				}
			}

			// Get the rect of the (hidden) view control, and convert to dialog coordinates
			RECT r;
			GetWindowRect(GetDlgItem(hdlg_, IDC_LENSVIEW), &r);
			r.right -= r.left;
			r.bottom -= r.top;
			MapWindowPoints(NULL, hdlg_, reinterpret_cast<POINT*>(&r), 1);

			// Stretch the memory bitmap created above to fill the view area
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hdlg_, &ps);
			StretchBlt(hdc, r.left, r.top, r.right, r.bottom, hdcMem, 0, 0, STRIPS, STRIP_HEIGHT, SRCCOPY);
			EndPaint(hdlg_, &ps); 

			// Select the old bitmap, and delete the bitmap and DC
			DeleteObject(SelectObject(hdcMem, hbmpOld));
			DeleteDC(hdcMem);

			// Release the screen DC
			ReleaseDC(NULL, hdcS);
			return 0L;
		}
	}

	return 0;
}


// Window procedure for the transparent selection window
LRESULT CALLBACK WndProc (HWND hwnd_, UINT uMsg_, WPARAM wParam_, LPARAM lParam_)
{
	static bool fDown = false;
	static POINT ptStart, ptLast;

	// Chances are this is a mouse message, so extract screen co-ordinates from it
	POINT pt = { GET_X_LPARAM(lParam_), GET_Y_LPARAM(lParam_) };
	ClientToScreen(hwnd_, &pt);

	switch (uMsg_)
	{
		case WM_CREATE:
			// No selection initially
			SetRectEmpty(&rView);
			return 0;

		case WM_SETCURSOR:
		{
			// Display a selection cross-hair
			static HCURSOR hcursor = LoadCursor(__hinstance, MAKEINTRESOURCE(IDC_CURSOR));
			SetCursor(hcursor);
			return TRUE;
		}

		case WM_LBUTTONDOWN:
			fDown = true;

			// Record the point and the window for the start of the drag
			ptLast = pt;
			rView.left = pt.x;
			rView.top  = pt.y;
			rView.right = rView.bottom = 0;

			// Restrict the mouse to the selection window area
			RECT rect;
			GetClientRect(hwnd_, &rect);
			MapWindowPoints(hwnd_, NULL, reinterpret_cast<POINT*>(&rect), 2);
			ClipCursor(&rect);
			break;

		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
		{
			if (fDown)
			{
				// Get the screen DC as set the drawing mode to XOR
				HDC hdc = GetDC(NULL);
				int nOldMode = SetROP2(hdc, R2_NOTXORPEN);

				// Create a red pen for the rubber hand
				HPEN hpenOld = (HPEN)SelectObject(hdc, CreatePen(PS_DOT, 0, RGB(0xff,0x00,0x00)));

				// Remove the previous rectangle
				Rectangle(hdc, rView.left, rView.top, ptLast.x+1, ptLast.y+1);

				// Mouse movement?
				if (uMsg_ == WM_MOUSEMOVE)
				{
					// Set the updated end point
					rView.right  = pt.x-rView.left+1;
					rView.bottom = pt.y-rView.top+1;

					// Draw a rectangle in the new position
					Rectangle(hdc, rView.left, rView.top, pt.x+1, pt.y+1);
					ptLast = pt;
				}

				// Restore the old mode and release the screen DC
				DeleteObject(SelectObject(hdc, hpenOld));
				SetROP2(hdc, nOldMode);
				ReleaseDC(NULL, hdc);

				// Left button up at the end of the selection?
				if (uMsg_ == WM_LBUTTONUP)
				{
					fDown = false;

					// Allow the mouse to roam freely and destroy the selection window
					ClipCursor(NULL);
					DestroyWindow(hwnd_);

					// Activate the window containing the start point (possibly an emulator)
					POINT pt = { rView.left, rView.top };
					SetForegroundWindow(WindowFromPoint(pt));
				}

				// Force the view to be updated
				InvalidateRect(g_hdlg, NULL, FALSE);
			}
			break;
		}
	}

	return DefWindowProc(hwnd_, uMsg_, wParam_, lParam_);
}


// We don't need any CRT functions for release, so we'll do things ourself!
extern "C" void __cdecl WinMainCRTStartup ()
{
	// Register a window class for the selection window
	WNDCLASS wc = {0};
	wc.lpszClassName = WINDOW_CLASS;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = __hinstance = GetModuleHandle(NULL);
	RegisterClass(&wc);

	// Show the main dialog
	DialogBox(__hinstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, DlgProc);

	ExitProcess(0);
}

#ifdef _DEBUG
// Use the CRT stack checking for debug
int __cdecl main (int argc_, char* argv_[])
{
	WinMainCRTStartup();
	return 0;
}
#endif
