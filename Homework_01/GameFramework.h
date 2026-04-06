// GameFramework.h
#pragma once


#include "Player.h"
#include "Scene.h"
#include "Timer.h"

class CGameFramework
{
public:
	CGameFramework() {}
	~CGameFramework() {}

private:
	// 윈도우 응용 프로그램의 인스턴스 핸들과 주 윈도우 핸들
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	// 주 윈도우의 클라이언트 사각형 영역
	RECT m_rcClient;

	// 렌더링 대상이 되는 화면에 해당하는 비트맵과 비트맵 디바이스 컨텍스트
	HDC m_hDCFrameBuffer = nullptr;
	HBITMAP m_hBitmapFrameBuffer = nullptr;
	HBITMAP m_hBitmapSelect = nullptr;

	// 플레이어 객체
	CPlayer* m_pPlayer = nullptr;
	// 게임 객체들을 포함하는 씬(게임 세계) 클래스
	CScene* m_pScene = nullptr;

	// 프레임 레이트를 관리하기 위한 타이머 클래스 객체
	CGameTimer m_GameTimer;

	//마지막으로 마우스 버튼을 클릭할 때의 마우스 커서의 위치이다.
	POINT m_ptOldCursorPos;

	//프레임 레이트를 출력하기 위한 문자열이다. 
	_TCHAR m_pszFrameRate[50];

public:
	// 프레임워크를 생성하는 함수(주 윈도우가 생성되면 호출)
	void OnCreate(HINSTANCE hInstance, HWND hMainWnd);

	// 프레임워크를 소멸하는 함수(응용 프로그램이 종료되면 호출)
	void OnDestroy();

	// 게임 세계를 렌더링할 비트맵 표면을 생성하고, 지우고, 클라이언트 영역으로 복사 
	void BuildFrameBuffer();
	void ClearFrameBuffer(DWORD dwColor);
	void PresentFrameBuffer();

	// 렌더링할 메쉬와 게임 객체를 생성하고 소멸하는 함수
	void BuildObjects();
	void ReleaseObjects();

	// 프레임워크의 핵심(사용자 입력, 애니메이션, 렌더링)을 구성하는 함수
	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	// 윈도우 메시지(키보드, 마우스)를 처리
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM
		wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM
		wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT
		nMessageID, WPARAM wParam, LPARAM lParam);
};

