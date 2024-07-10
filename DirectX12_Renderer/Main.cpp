#include "Window.h"
#include "Scene.h"
#include <windowsx.h>

using namespace std;
using namespace graphics;
using namespace window;

static const LPCWSTR	appName = L"Directx 12 Moon Renderer";
static const int		WINDOW_HEIGHT = 540;
static const int		WINDOW_WIDTH = 960;
static const bool		FULL_SCREEN = false;

static Scene* pScene = nullptr;
static int lastMouseX = -1;
static int lastMouseY = -1;

InputDirections g_inputDirections;

static void HandleMouseMove(LPARAM lp) {
	int x = GET_X_LPARAM(lp);
	int y = GET_Y_LPARAM(lp);

	if (lastMouseX == -1) { // then we haven't actually moved the mouse yet, we're just starting. 
		// do nothing
	}
	else { // calculate how far we've moved from the last time we updated and pass that info to the scene.
		// clamp the values as well so that when we're in windowed mode, we don't cause massive jumps when the
		// mouse moves out of the window and then back in at a completely different position.
		float moveX = lastMouseX - x;
		moveX = moveX > 20 ? 20 : moveX;
		moveX = moveX < -20 ? -20 : moveX;
		float moveY = lastMouseY - y;
		moveY = moveY > 20 ? 20 : moveY;
		moveY = moveY < -20 ? -20 : moveY;

		pScene->HandleMouseInput(moveX, moveY);
	}

	// update our last mouse coordinates;
	lastMouseX = x;
	lastMouseY = y;
}

static LRESULT CALLBACK WndProc(HWND win , UINT msg, WPARAM wp, LPARAM lp) {
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(win, msg, wp, lp)) { 
		return true;
	}
	else {
		switch (msg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			exit(100);
			return 0;

		case WM_CLOSE:
			DestroyWindow(win);
			return 0;
			
        case WM_MOUSEMOVE:
            if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow) && (wp & MK_RBUTTON)) {
                HandleMouseMove(lp);
            }
			else {
				lastMouseX = GET_X_LPARAM(lp);
				lastMouseY = GET_Y_LPARAM(lp);
			}
            break;

		case WM_KEYDOWN:
			switch (wp)
			{
			case 'W':
				g_inputDirections.bFront = true;
				break;

			case 'S':
				g_inputDirections.bBack = true;
				break;

			case 'A':
				g_inputDirections.bLeft = true;
				break;

			case 'D':
				g_inputDirections.bRight = true;
				break;

			case 'E':
				g_inputDirections.bUp = true;
				break;
			case 'Q':
				g_inputDirections.bDown = true;
				break;
			case '1':
				g_inputDirections.bMode1 = true;
				break;
			case '2':
				g_inputDirections.bMode2 = true;
				break;
			default:
				break;
			}
			break;

		case WM_KEYUP:
			switch (wp)
			{
			case 'W':
				g_inputDirections.bFront = false;
				break;

			case 'S':
				g_inputDirections.bBack = false;
				break;

			case 'A':
				g_inputDirections.bLeft = false;
				break;

			case 'D':
				g_inputDirections.bRight = false;
				break;

			case 'E':
				g_inputDirections.bUp = false;
				break;

			case 'Q':
				g_inputDirections.bDown = false;
				break;
			case '1':
				g_inputDirections.bMode1 = false;
				break;
			case '2':
				g_inputDirections.bMode2 = false;
				break;
			default:
				break;
			}
			break;

		default:
			return DefWindowProc(win, msg, wp, lp);
		}
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, PSTR cmdLine, int cmdShow) {
	try {
		LARGE_INTEGER startTime;
		LARGE_INTEGER endTime;
		LARGE_INTEGER frequency;
		LARGE_INTEGER elapsedMsc;

		QueryPerformanceCounter(&startTime);
		QueryPerformanceFrequency(&frequency);

		Window WIN(appName, WINDOW_HEIGHT, WINDOW_WIDTH, WndProc, FULL_SCREEN);
		Graphics Renderer(WIN.Height(), WIN.Width(), WIN.GetWindow(), FULL_SCREEN);
		Scene MainScene(WIN.Height(), WIN.Width(), &Renderer);
		pScene = &MainScene;
	
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));

		while (1) {
			if (PeekMessage(&msg, WIN.GetWindow(), 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			} 
			if (msg.message == WM_QUIT) { 
				pScene = nullptr;
				return 1;
			}
			QueryPerformanceCounter(&endTime);
			elapsedMsc.QuadPart = endTime.QuadPart - startTime.QuadPart;
			elapsedMsc.QuadPart *= 1000000;
			elapsedMsc.QuadPart /= frequency.QuadPart;
			QueryPerformanceFrequency(&frequency);
			QueryPerformanceCounter(&startTime);
			FLOAT deltaTime = static_cast<FLOAT>(elapsedMsc.QuadPart) / 1000000.0f;

			MainScene.HandleInput(g_inputDirections, deltaTime);
			MainScene.Draw();

		}

		pScene = nullptr;
		return 2;
	} catch (GFX_Exception& e) {
		OutputDebugStringA(e.what());
		pScene = nullptr;
		return 3;
	} catch (Window_Exception& e) {
		OutputDebugStringA(e.what());
		pScene = nullptr;
		return 4;
	}
}