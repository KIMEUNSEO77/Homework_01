// Scene.h
#pragma once
#include <vector>
#include "GameObject.h"
#include "Camera.h"
#include "Player.h"
#include "Bullet.h"

class CScene
{
public:
	CScene(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	virtual ~CScene() {}

private:
	// 게임 객체들의 개수와 개임 객체들의 리스트
	int m_nObjects = 0;
	CGameObject** m_ppObjects = nullptr;

	CPlayer* m_pPlayer = nullptr;

	std::vector<CBullet*> m_Bullets;  // 총알 리스트

public:
	// 게임 객체들을 생성하고 소멸
	virtual void BuildObjects();
	virtual void ReleaseObjects();

	// 게임 객체들을 애니메이션
	virtual void Animate(float fElapsedTime);

	// 게임 객체들을 렌더링
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

	// 총알 생성
	void CreateBullet(const XMFLOAT3& xmf3Position, const XMFLOAT3& xmf3Direction);

	// 충돌 체크 함수
	void CheckBulletCollisions();


	// 윈도우 메시지(키보드, 마우스)를 처리
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID,
		WPARAM wParam, LPARAM lParam) { }
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID,
		WPARAM wParam, LPARAM lParam) { }
};

