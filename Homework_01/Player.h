// Player.h


#pragma once

#include "GameObject.h"
#include "Camera.h"

// 플레이어 객체도 게임 객체이기 때문에 CGameObject 클래스를 상속받음
class CPlayer : public CGameObject
{
public:
	CPlayer() {}
	virtual ~CPlayer() { if (m_pCamera) delete m_pCamera; }
	
public:
	// 플레이어의 위치 벡터, x-축(Right), y-축(Up), z-축(Look) 벡터
	XMFLOAT3 m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3 m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3 m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	// 3인칭 카메라의 오프셋(Offset) 벡터
	XMFLOAT3 m_xmf3CameraOffset = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// 플레이어의 이동 속도를 나타내는 벡터
	XMFLOAT3 m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// 플레이어에 작용하는 마찰력
	float m_fFriction = 125.0f;

	// 플레이어가 로컬 x-축(Right), y-축(Up), z-축(Look)으로 얼마만큼 회전했는가
	float m_fPitch = 0.0f;
	float m_fYaw = 0.0f;
	float m_fRoll = 0.0f;

	// 플레이어 객체에 포함된 카메라
	CCamera* m_pCamera = nullptr;

public:
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);

	// 플레이어가 어떤 지점을 향하도록 하는 함수
	void LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up);

	// 플레이어를 이동하는 함수
	void Move(DWORD dwDirection, float fDistance);
	void Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity);
	void Move(float x, float y, float z);

	// 플레이어를 회전하는 함수
	void Rotate(float fPitch, float fYaw, float fRoll);
	void SetCameraOffset(const XMFLOAT3& xmf3CameraOffset);

	// 플레이어의 위치와 회전 정보를 경과 시간에 따라 갱신하는 함수
	void Update(float fTimeElapsed = 0.016f);

	// 플레이어의 위치와 회전축으로부터 월드 변환 행렬을 생성하는 함수
	virtual void OnUpdateTransform();
	virtual void Animate(float fElapsedTime);

	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }
	CCamera* GetCamera() { return(m_pCamera); }
};

class CAirplanePlayer : public CPlayer
{
public:
	CAirplanePlayer();
	virtual ~CAirplanePlayer();
	virtual void OnUpdateTransform();
};

