// LensKey - a Lenslok Emulator
//
// (c) 2002-2026 Simon Owen <simon@simonowen.com>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <array>

#include "resource.h"

namespace
{

constexpr auto LENS_STRIPS{ 16 };	// Vertical strips in the lens viewer
constexpr auto STRIP_HEIGHT{ 32 };	// Sample strips taken are stored as 32-pixels high
constexpr auto right_edge_percent{ 100 };
constexpr auto border_percent{ 101 };

constexpr auto WINDOW_CLASS{ "LensKeyClass" };

constexpr auto game_names{ std::to_array(
	{
		"ACE (Air Combat Emulator)",
		"Art Studio",
		"Elite",
		"Graphic Adventure Creator",
		"Jewels of Darkness",
		"Mooncresta",
		"Price of Magik",
		"Tomahawk",
		"TT Racer"
	}) };

// 16 strip values: 6 for each character, with padding to left arnd right of each character.
// Values indicate the position a strip is taken from as a percentage of the selection width.
// Negative values are to the left of the selection, positive to the right.
const std::array<std::array<int, LENS_STRIPS>, 9> game_decodes =
{{
	{{ 0, -81, -31,  13, -62, -41,  22, 0,  0, -22,  39, 58, -12, 29, 70, 0 }},	// ACE
	{{ 0, -41, -30, -68, -52, -11, -20, 0,  0,  32,  60, 11,  22, 49, 71, 0 }},	// Art Studio
	{{ 0, -41, -57, -77,  10, -28, -19, 0,  0,  43, -10, 22,  32, 77, 58, 0 }},	// Elite
	{{ 0, -77, -28,  -4, -19, -59, -39, 0,  0,  20,  51, 10,  10, 66, 28, 0 }},	// Graphic Adventure Creator
	{{ 0, -40, -57, -71,  14, -27, -21, 0,  0,  42, -12, 22,  27, 67, 53, 0 }},	// Jewels of Darkness
	{{ 0, -79, -31,  -7, -22, -61, -44, 0,  0,  18,  50,  7,  67, 39, 27, 0 }},	// Mooncresta
	{{ 0, -27, -39, -71, -6,  -17, -48, 0,  0,  51,  64,  7,  40, 17, 79, 0 }},	// Price of Magik
	{{ 0, -82, -31, -58, -20, -42,  10, 0,  0, -10,  32, 65,  20, 44, 80, 0 }},	// Tomahawk
	{{ 0, -20, -41, -69, -53,   6, -29, 0,  0,  -9,  64, 20,  46, 33, 81, 0 }}	// TT Racer
}};

HINSTANCE __hinstance;
HWND g_hdlg;
RECT rView;


auto CALLBACK DlgProc(HWND hdlg_, UINT uMsg_, WPARAM wParam_, LPARAM /*lParam_*/) -> INT_PTR
{
	static UINT_PTR nTimer;

	switch (uMsg_)
	{
		case WM_INITDIALOG:
		{
			// Set the viewer window as top-most, so it's always visible
			SetWindowPos(g_hdlg = hdlg_, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE);

			constexpr auto ms_per_second{ 1000 };
			constexpr auto updates_per_second{ 10 };
			nTimer = SetTimer(hdlg_, 1, ms_per_second / updates_per_second, nullptr);

			HWND hwndCombo{ GetDlgItem(hdlg_, IDC_TITLE) };
			for (const auto& game : game_names)
			{
				ComboBox_AddString(hwndCombo, game);
			}
			ComboBox_SetCurSel(hwndCombo, 0);
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
			InvalidateRect(hdlg_, nullptr, FALSE);
			return 0;

		case WM_LBUTTONDOWN:
		{
			// Fetch the size of the work area, which excludes the taskbar
			RECT rect;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

			// Create the transparent selection window, filling the entire work area
			CreateWindowEx(WS_EX_TRANSPARENT|WS_EX_TOPMOST, WINDOW_CLASS, "", WS_POPUP|WS_VISIBLE,
							rect.left, rect.top, rect.right, rect.bottom, g_hdlg, nullptr, __hinstance, nullptr);
			break;
		}

		case WM_PAINT:
		{
			HDC hdc_screen{ GetDC(nullptr) };

			HDC hdc_mem = CreateCompatibleDC(hdc_screen);
			auto* hbmp_old = SelectObject(hdc_mem, CreateCompatibleBitmap(hdc_screen, LENS_STRIPS, STRIP_HEIGHT));
			BitBlt(hdc_mem, 0, 0, LENS_STRIPS, STRIP_HEIGHT, nullptr, 0, 0, WHITENESS);

			// The decode table used depends on the title combo box selection
			auto game_index{ SendDlgItemMessage(hdlg_, IDC_TITLE, CB_GETCURSEL, 0, 0L) };
			const auto& game_decode = game_decodes.at(game_index);

			// Have we got anything to draw?
			if (rView.right > 1)
			{
				// Decode the image by taking vertical strips at the appropriate positions
				for (int i = 0 ; i < LENS_STRIPS ; i++)
				{
					// Work out the pixel offset, using an offset off the end for zero (spacing gap)
					int nOffset = MulDiv(rView.right, (game_decode.at(i) != 0) ?
                                         game_decode.at(i) : border_percent, right_edge_percent);

					// Copy a slice to our memory bitmap
					StretchBlt(hdc_mem, i, 0, 1, STRIP_HEIGHT, hdc_screen, rView.left+nOffset, rView.top+1, 1, rView.bottom-2, SRCCOPY);
				}
			}

			// Get the rect of the (hidden) view control, and convert to dialog coordinates
			RECT rect;
			GetWindowRect(GetDlgItem(hdlg_, IDC_LENSVIEW), &rect);
			MapWindowRect(nullptr, hdlg_, &rect);
			rect.right -= rect.left;
			rect.bottom -= rect.top;

			// Stretch the memory bitmap created above to fill the view area
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hdlg_, &ps);
			StretchBlt(hdc, rect.left, rect.top, rect.right, rect.bottom, hdc_mem, 0, 0, LENS_STRIPS, STRIP_HEIGHT, SRCCOPY);
			EndPaint(hdlg_, &ps);

			// Select the old bitmap, and delete the bitmap and DC
			DeleteObject(SelectObject(hdc_mem, hbmp_old));
			DeleteDC(hdc_mem);

			// Release the screen DC
			ReleaseDC(nullptr, hdc_screen);
			return 0L;
		}
	}

	return 0;
}


auto CALLBACK TransparentWndProc (HWND hwnd_, UINT uMsg_, WPARAM wParam_, LPARAM lParam_) -> LRESULT
{
	static bool mouse_drag = false;
	static POINT ptLast;

	// Chances are this is a mouse message, so extract screen co-ordinates from it
	POINT pt{ GET_X_LPARAM(lParam_), GET_Y_LPARAM(lParam_) };
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
		{
			mouse_drag = true;

			// Record the point and the window for the start of the drag
			ptLast = pt;
			rView.left = pt.x;
			rView.top  = pt.y;
			rView.right = rView.bottom = 0;

			// Restrict the mouse to the selection window area
			RECT rect;
			GetClientRect(hwnd_, &rect);
			MapWindowRect(hwnd_, nullptr, &rect);
			ClipCursor(&rect);
			break;
		}

		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
		{
			if (mouse_drag)
			{
				constexpr auto line_vert_offset{ 8 };

				// The decode table used depends on the title combo box selection
				const auto& panDecode = game_decodes.at(SendDlgItemMessage(GetParent(hwnd_), IDC_TITLE, CB_GETCURSEL, 0, 0L));

				// Get the screen DC as set the drawing mode to XOR
				HDC hdc = GetDC(nullptr);
				int nOldMode = SetROP2(hdc, R2_NOTXORPEN);

				// Create a red pen for the rubber band
				HPEN hpenRect = CreatePen(PS_DOT, 0, RGB(0xff,0x00,0x00));
				HPEN hpenLine = CreatePen(PS_SOLID, 0, RGB(0xff,0x00,0x00));

				// Remove the previous rectangle
				auto* hpenOld = static_cast<HPEN>(SelectObject(hdc, hpenRect));
				Rectangle(hdc, rView.left, rView.top, ptLast.x+1, ptLast.y+1);

				// Remove the previous strip markers
				SelectObject(hdc, hpenLine);
				for (int i = 0 ; i < LENS_STRIPS ; i++)
				{
					int nOffset = MulDiv(rView.right, (panDecode.at(i) != 0) ?
                                         panDecode.at(i) : border_percent, right_edge_percent);
					MoveToEx(hdc, rView.left+nOffset, rView.top-line_vert_offset, nullptr);
					LineTo(hdc, rView.left+nOffset, rView.top);
				}

				// Mouse movement?
				if (uMsg_ == WM_MOUSEMOVE)
				{
					// Set the updated end point
					rView.right  = pt.x-rView.left+1;
					rView.bottom = pt.y-rView.top+1;

					// Draw a rectangle in the new position
					SelectObject(hdc, hpenRect);
					Rectangle(hdc, rView.left, rView.top, pt.x+1, pt.y+1);

					// Draw the new strip markers
					SelectObject(hdc, hpenLine);
					for (int i = 0 ; i < LENS_STRIPS ; i++)
					{
						int nOffset = MulDiv(rView.right, (panDecode.at(i) != 0) ?
                                             panDecode.at(i) : border_percent, right_edge_percent);
						MoveToEx(hdc, rView.left+nOffset, rView.top-line_vert_offset, nullptr);
						LineTo(hdc, rView.left+nOffset, rView.top);
					}

					// Remember the new end position so we can erase the rectangle next time
					ptLast = pt;
				}

				// Restore the old mode and release the screen DC
				SelectObject(hdc, hpenOld);
				DeleteObject(hpenRect);
				DeleteObject(hpenLine);
				SetROP2(hdc, nOldMode);
				ReleaseDC(nullptr, hdc);

				// Left button up at the end of the selection?
				if (uMsg_ == WM_LBUTTONUP)
				{
					mouse_drag = false;

					// Allow the mouse to roam freely and destroy the selection window
					ClipCursor(nullptr);
					DestroyWindow(hwnd_);

					// Activate the window containing the start point (possibly an emulator)
					pt = { .x=rView.left, .y=rView.top };
					SetForegroundWindow(WindowFromPoint(pt));
				}

				// Force the view to be updated
				InvalidateRect(g_hdlg, nullptr, FALSE);
			}
			break;
		}
	}

	return DefWindowProc(hwnd_, uMsg_, wParam_, lParam_);
}

} // namespace


auto WinMain(
	HINSTANCE hInstance,
	HINSTANCE /*hPrevInstance*/,
	LPSTR /*lpCmdLine*/,
	int /*nShowCmd*/) -> int
{
	WNDCLASS wc{};
	wc.lpfnWndProc = TransparentWndProc;
	wc.hInstance = __hinstance = GetModuleHandle(nullptr);
	wc.lpszClassName = WINDOW_CLASS;
	RegisterClass(&wc);

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), nullptr, DlgProc);
	return 0;
}
