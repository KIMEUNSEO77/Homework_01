// Camera.cpp


#include "stdafx.h"
#include "Camera.h"
#include "Player.h"

void CViewport::SetViewport(int nLeft, int nTop, int nWidth, int
	nHeight)
{
	m_nLeft = nLeft;
	m_nTop = nTop;
	m_nWidth = nWidth;
	m_nHeight = nHeight;
}

// 카메라가 여러 번 회전을 하게 되면 누적된 실수 표현의 문제 때문에 
// 카메라의 로컬 x-축(Right), y축(Up), z-축(Look)이 서로 직교하지 않을 수 있음
// 카메라의 로컬 x-축(Right), y-축(Up), z-축(Look)이 서로 직교하도록 만들어줌
void CCamera::GenerateViewMatrix()
{
	// 카메라의 z-축을 기준으로 카메라의 좌표축들이 직교하도록 만듦
	XMVECTOR xmvLook = XMVector3Normalize(XMLoadFloat3(&m_xmf3Look));
	XMVECTOR xmvUp = XMVector3Normalize(XMLoadFloat3(&m_xmf3Up));
	XMVECTOR xmvRight = XMVector3Normalize(XMVector3Cross(xmvUp, xmvLook));

	xmvUp = XMVector3Normalize(XMVector3Cross(xmvLook, xmvRight));

	XMStoreFloat3(&m_xmf3Look, xmvLook);
	XMStoreFloat3(&m_xmf3Right, xmvRight);
	XMStoreFloat3(&m_xmf3Up, xmvUp);

	// 카메라 변환 행렬은 카메라 월드변환 행렬의 역행렬(전치행렬)
	m_xmf4x4View._11 = m_xmf3Right.x; m_xmf4x4View._12 = m_xmf3Up.x; m_xmf4x4View._13 = m_xmf3Look.x;
	m_xmf4x4View._21 = m_xmf3Right.y; m_xmf4x4View._22 = m_xmf3Up.y; m_xmf4x4View._23 = m_xmf3Look.y;
	m_xmf4x4View._31 = m_xmf3Right.z; m_xmf4x4View._32 = m_xmf3Up.z; m_xmf4x4View._33 = m_xmf3Look.z;

	XMVECTOR xmvPosition = XMLoadFloat3(&m_xmf3Position);
	m_xmf4x4View._41 = -XMVectorGetX(XMVector3Dot(xmvPosition, xmvRight));
	m_xmf4x4View._42 = -XMVectorGetX(XMVector3Dot(xmvPosition, xmvUp));
	m_xmf4x4View._43 = -XMVectorGetX(XMVector3Dot(xmvPosition, xmvLook));

	XMStoreFloat4x4(&m_xmf4x4ViewProject, XMMatrixMultiply(XMLoadFloat4x4(&m_xmf4x4View), XMLoadFloat4x4(&m_xmf4x4Project)));
}

// 카메라가 xmf3LookAt을 바라보도록 카메라 변환 행렬을 갱신
void CCamera::SetLookAt(XMFLOAT3& xmf3Position, XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up)
{
	m_xmf3Position = xmf3Position;

	XMStoreFloat4x4(&m_xmf4x4View,
		XMMatrixLookAtLH(XMLoadFloat3(&m_xmf3Position), XMLoadFloat3(&xmf3LookAt), XMLoadFloat3(&xmf3Up)));

	// 카메라 변환 행렬에서 카메라의 x-축, y-축, z-축을 구함
	XMVECTORF32 xmf32vRight = { m_xmf4x4View._11, m_xmf4x4View._21, m_xmf4x4View._31, 0.0f };
	XMVECTORF32 xmf32vUp = { m_xmf4x4View._12, m_xmf4x4View._22, m_xmf4x4View._32, 0.0f };
	XMVECTORF32 xmf32vLook = { m_xmf4x4View._13, m_xmf4x4View._23, m_xmf4x4View._33, 0.0f };

	XMStoreFloat3(&m_xmf3Right, XMVector3Normalize(xmf32vRight));
	XMStoreFloat3(&m_xmf3Up, XMVector3Normalize(xmf32vUp));
	XMStoreFloat3(&m_xmf3Look, XMVector3Normalize(xmf32vLook));
}

void CCamera::SetLookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up)
{
	// 현재 카메라의 위치에서 플레이어를 바라보기 위한 카메라 변환 행렬을 생성
	SetLookAt(m_xmf3Position, xmf3LookAt, xmf3Up);
}

void CCamera::SetViewport(int nLeft, int nTop, int nWidth, int nHeight)
{
	m_Viewport.SetViewport(nLeft, nTop, nWidth, nHeight);
	m_fAspectRatio = float(m_Viewport.m_nWidth) / float(m_Viewport.m_nHeight);
}

void CCamera::SetFOVAngle(float fFOVAngle)
{
	m_fFOVAngle = fFOVAngle;
	m_fProjectRectDistance = float(1.0f / tan(XMConvertToRadians(fFOVAngle * 0.5f)));
}

// 원근 투영 변환 행렬을 생성
void CCamera::GeneratePerspectiveProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fFOVAngle)
{
	float fAspectRatio = (float(m_Viewport.m_nWidth) / float(m_Viewport.m_nHeight));

	XMStoreFloat4x4(&m_xmf4x4Project,
		XMMatrixPerspectiveFovLH(XMConvertToRadians(fFOVAngle), 
			fAspectRatio, fNearPlaneDistance, fFarPlaneDistance));
}

// 카메라를 이동
void CCamera::Move(const XMFLOAT3& xmf3Shift)
{
	XMStoreFloat3(&m_xmf3Position,
		XMVectorAdd(XMLoadFloat3(&m_xmf3Position), XMLoadFloat3(&xmf3Shift)));
}

void CCamera::Move(float x, float y, float z)
{
	Move(XMFLOAT3(x, y, z));
}

// 카메라의 로컬 x-축(Right), y-축(Up), z-축(Look)을 기준으로 회전하는 함수
void CCamera::Rotate(float fPitch, float fYaw, float fRoll)
{
	if (fPitch != 0.0f)
	{
		XMMATRIX xmmtxRotate =
			XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(fPitch));

		// 카메라의 로컬 x-축(Right)을 중심으로 회전하는 행렬로 y-축(Up), z-축(Look)을 회전
		XMStoreFloat3(&m_xmf3Look,
			XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Look), xmmtxRotate));
		XMStoreFloat3(&m_xmf3Up,
			XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Up), xmmtxRotate));
	}
	if (fYaw != 0.0f)
	{
		XMMATRIX xmmtxRotate =
			XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up),
				XMConvertToRadians(fYaw));
		XMStoreFloat3(&m_xmf3Look,
			XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Look), xmmtxRotate));
		XMStoreFloat3(&m_xmf3Right,
			XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Right), xmmtxRotate));
	}
	if (fRoll != 0.0f)
	{
		XMMATRIX xmmtxRotate =
			XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look),
				XMConvertToRadians(fRoll));
		XMStoreFloat3(&m_xmf3Up,
			XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Up), xmmtxRotate));
		XMStoreFloat3(&m_xmf3Right,
			XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Right), xmmtxRotate));
	}
}

void CCamera::Update(CPlayer* pPlayer, XMFLOAT3& xmf3LookAt, float fTimeElapsed)
{
	// 플레이어의 로컬 x-축, y-축, z-축 벡터로부터 회전 행렬(플레이어와 같은 방향을 나타내는 행렬)을 생성
		XMMATRIX xmmtx4Rotate;
	xmmtx4Rotate.r[0] = XMVectorSet(pPlayer->m_xmf3Right.x,
		pPlayer->m_xmf3Right.y, pPlayer->m_xmf3Right.z, 0.0f);
	xmmtx4Rotate.r[1] = XMVectorSet(pPlayer->m_xmf3Up.x,
		pPlayer->m_xmf3Up.y, pPlayer->m_xmf3Up.z, 0.0f);
	xmmtx4Rotate.r[2] = XMVectorSet(pPlayer->m_xmf3Look.x,
		pPlayer->m_xmf3Look.y, pPlayer->m_xmf3Look.z, 0.0f);
	xmmtx4Rotate.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	XMVECTOR xmvPosition = XMLoadFloat3(&m_xmf3Position);
	// 카메라 오프셋 벡터를 회전 행렬로 변환(회전)
	XMVECTOR xmvOffset =
		XMVector3TransformCoord(XMLoadFloat3(&pPlayer->m_xmf3CameraOffset),
			xmmtx4Rotate);

	// 회전한 카메라의 위치는 플레이어의 위치에 회전한 카메라 오프셋 벡터를 더한 것
	XMVECTOR xmvNewPosition =
		XMVectorAdd(XMLoadFloat3(&pPlayer->m_xmf3Position), xmvOffset);

	// 현재의 카메라의 위치에서 회전한 카메라의 위치까지의 방향과 거리를 나타내는 벡터
	XMVECTOR xmvDirection = XMVectorSubtract(xmvNewPosition, xmvPosition);
	float fLength = XMVectorGetX(XMVector3Length(xmvDirection));
	xmvDirection = XMVector3Normalize(xmvDirection);

	// 카메라의 래그(Lag)는 플레이어가 회전하더라도 카메라가 동시에 따라서 회전하지 않고 
	// 약간의 시차를 두고 회전하는 효과를 구현하기 위한 것.
	float fTimeLagScale = fTimeElapsed * 4.0f;
	float fDistance = fLength * fTimeLagScale;

	if (fDistance > fLength) fDistance = fLength;
	if (fLength < 0.01f) fDistance = fLength;
	if (fDistance > 0)
	{
		// 카메라를 공전하지 않고 이동 (회전의 각도가 작은 경우 회전 이동은 선형 이동과 거의 같음)
		XMStoreFloat3(&m_xmf3Position, XMVectorAdd(xmvPosition,
			XMVectorScale(xmvDirection, fDistance)));
		// 카메라가 플레이어를 바라보도록 함
		SetLookAt(pPlayer->m_xmf3Position, pPlayer->m_xmf3Up);
	}
}
