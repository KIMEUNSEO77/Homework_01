// EnemyObject.cpp
#include "stdafx.h"
#include "GameObject.h"
#include "Player.h"
#include "GraphicsPipeline.h"
#include "EnemyObject.h"

void CEnemyObject::Animate(float fElapsedTime)
{
	UpdateLookAtPlayer();

	// 공격 타이머 
	m_fAttackElapsed += fElapsedTime;

	if (m_fAttackElapsed >= m_fAttackInterval)
	{
		m_bFireBullet = true;
		m_fAttackElapsed = 0.0f;
	}
}

// 공격 위치 구하는 함수
XMFLOAT3 CEnemyObject::GetMuzzleWorldPosition() const
{
	XMFLOAT3 pos = GetPosition();

	// Look 벡터 이용
	XMFLOAT3 look = XMFLOAT3(
		m_xmf4x4World._31,
		m_xmf4x4World._32,
		m_xmf4x4World._33
	);

	pos.x += look.x * 2.5f;
	pos.y += look.y * 2.5f;
	pos.z += look.z * 2.5f;

	return pos;
}

void CEnemyObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	// 본체 렌더
	CGameObject::Render(hDCFrameBuffer, pCamera);

	// 총구 렌더
	XMFLOAT3 muzzlePos = GetMuzzleWorldPosition();

	XMFLOAT4X4 xmf4x4Muzzle = m_xmf4x4World;
	xmf4x4Muzzle._41 = muzzlePos.x;
	xmf4x4Muzzle._42 = muzzlePos.y;
	xmf4x4Muzzle._43 = muzzlePos.z;

	// 스케일 축소
	XMMATRIX xmScale = XMMatrixScaling(0.25f, 0.25f, 0.25f);
	XMMATRIX xmWorld = XMLoadFloat4x4(&xmf4x4Muzzle);
	XMMATRIX xmFinal = XMMatrixMultiply(xmScale, xmWorld);

	XMFLOAT4X4 xmf4x4Final;
	XMStoreFloat4x4(&xmf4x4Final, xmFinal);

	CGraphicsPipeline::SetWorldTransform(&xmf4x4Final);

	HPEN hPen = ::CreatePen(PS_SOLID, 0, RGB(255, 0, 0));
	HBRUSH hBrush = ::CreateSolidBrush(RGB(255, 0, 0));

	HPEN hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hDCFrameBuffer, hBrush);

	static CCubeMesh s_MuzzleMesh(4.0f, 4.0f, 4.0f);
	s_MuzzleMesh.Render(hDCFrameBuffer);

	::SelectObject(hDCFrameBuffer, hOldBrush);
	::SelectObject(hDCFrameBuffer, hOldPen);
	::DeleteObject(hBrush);
	::DeleteObject(hPen);
}

XMFLOAT3 CEnemyObject::GetLookVector() const
{
	XMFLOAT3 look(
		m_xmf4x4World._31,
		m_xmf4x4World._32,
		m_xmf4x4World._33
	);

	XMVECTOR xmvLook = XMVector3Normalize(XMLoadFloat3(&look));
	XMStoreFloat3(&look, xmvLook);

	return look;
}

// 플레이어를 바라보도록
void CEnemyObject::UpdateLookAtPlayer()
{
	if (!m_pPlayer) return;

	XMFLOAT3 xmf3PlayerPosition = m_pPlayer->GetPosition();
	XMFLOAT3 xmf3MyPosition = GetPosition();

	XMFLOAT3 xmf3Look(
		xmf3PlayerPosition.x - xmf3MyPosition.x,
		0.0f, // 수평 회전만 하도록 y는 제거
		xmf3PlayerPosition.z - xmf3MyPosition.z
	);

	XMVECTOR xmvLook = XMLoadFloat3(&xmf3Look);
	xmvLook = XMVector3Normalize(xmvLook);
	XMStoreFloat3(&xmf3Look, xmvLook);

	// 월드 업 벡터
	XMFLOAT3 xmf3Up(0.0f, 1.0f, 0.0f);

	// Right = Up x Look
	XMVECTOR xmvRight = XMVector3Cross(XMLoadFloat3(&xmf3Up), XMLoadFloat3(&xmf3Look));
	xmvRight = XMVector3Normalize(xmvRight);

	XMFLOAT3 xmf3Right;
	XMStoreFloat3(&xmf3Right, xmvRight);

	// 다시 Up 보정 = Look x Right
	XMVECTOR xmvNewUp = XMVector3Cross(XMLoadFloat3(&xmf3Look), XMLoadFloat3(&xmf3Right));
	xmvNewUp = XMVector3Normalize(xmvNewUp);

	XMFLOAT3 xmf3NewUp;
	XMStoreFloat3(&xmf3NewUp, xmvNewUp);

	// 회전축 갱신
	m_xmf4x4World._11 = xmf3Right.x;
	m_xmf4x4World._12 = xmf3Right.y;
	m_xmf4x4World._13 = xmf3Right.z;

	m_xmf4x4World._21 = xmf3NewUp.x;
	m_xmf4x4World._22 = xmf3NewUp.y;
	m_xmf4x4World._23 = xmf3NewUp.z;

	m_xmf4x4World._31 = xmf3Look.x;
	m_xmf4x4World._32 = xmf3Look.y;
	m_xmf4x4World._33 = xmf3Look.z;
}