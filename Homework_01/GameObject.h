// GameObject.h

#pragma once

#include "Mesh.h"
#include "Camera.h"

class CGameObject
{
public:
	CGameObject() {}
	virtual ~CGameObject();

public:
	bool m_bActive = true;

	// 게임 객체의 모양(메쉬, 모델)
	CMesh* m_pMesh = NULL;

	// 게임 객체의 월드 변환 행렬 (위치, 회전, 크기 변환을 포함하는 행렬)
	XMFLOAT4X4 m_xmf4x4World = Matrix4x4::Identity();

	// 게임 객체의 색상(선분의 색상)
	DWORD m_dwColor = RGB(255, 0, 0);

	// 게임 객체의 이동 방향을 나타내는 벡터
	XMFLOAT3 m_xmf3MovingDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float m_fMovingSpeed = 0.0f;
	float m_fMovingRange = 0.0f;

	// 게임 객체의 회전축을 나타내는 벡터
	XMFLOAT3 m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	float m_fRotationSpeed = 0.0f;

	float m_fCollisionRadius = 1.0f;   // 충돌 반지름

protected:
	// BoundingSphere
	BoundingSphere m_xmBoundingSphere = BoundingSphere(XMFLOAT3(0.0f, 0.0f, 0.0f), 1.0f);
	
public:
	XMFLOAT3 GetPosition() const
	{ return XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43); }

	void SetMesh(CMesh* pMesh) {
		m_pMesh = pMesh; 
		if (pMesh)
			pMesh->AddRef();
	}
	void SetActive(bool bActive) { m_bActive = bActive; }
	void SetColor(DWORD dwColor) { m_dwColor = dwColor; }

	void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& xmf3Position);

	void SetMovingDirection(const XMFLOAT3& xmf3MovingDirection);
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
	void SetMovingRange(float fRange) { m_fMovingRange = fRange; }

	void SetRotationAxis(const XMFLOAT3& xmf3RotationAxis);
	void SetRotationSpeed(float fSpeed) { m_fRotationSpeed = fSpeed; }

	void Move(XMFLOAT3& vDirection, float fSpeed);

	void Rotate(float fPitch, float fYaw, float fRoll);
	void Rotate(XMFLOAT3& xmf3Axis, float fAngle);

	virtual void OnUpdateTransform() {}
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

	void SetCollisionRadius(float fRadius);
	float GetCollisionRadius() const { return m_fCollisionRadius; }

	// BoundingSphere 생성
	BoundingSphere GetBoundingSphere() const { return m_xmBoundingSphere; }
	void UpdateBoundingSphere();

	bool IsActive() { return m_bActive; }	
};


// 적 추적용 큐브
class CChasingCube : public CGameObject
{
public:
	CChasingCube() {}
	virtual ~CChasingCube() {}

	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	void SetChaseSpeed(float fSpeed) { m_fChaseSpeed = fSpeed; }

	virtual void Animate(float fElapsedTime) override;

private:
	CPlayer* m_pPlayer = nullptr;
	float m_fChaseSpeed = 0.0f; 
};

