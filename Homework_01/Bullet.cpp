// Bullet.cpp
#include "stdafx.h"
#include "GameObject.h"
#include "Bullet.h"

CBullet::CBullet()
{
}

CBullet::~CBullet()
{
}

void CBullet::SetDirection(const XMFLOAT3& xmf3Direction)
{
    XMStoreFloat3(&m_xmf3Direction, XMVector3Normalize(XMLoadFloat3(&xmf3Direction)));
}

void CBullet::SetSpeed(float fSpeed)
{
    m_fSpeed = fSpeed;
}

void CBullet::SetRange(float fRange)
{
    m_fRange = fRange;
}

void CBullet::SetVerticalVelocity(float fVelocity)
{
    m_fVerticalVelocity = fVelocity;
}

void CBullet::SetGravity(float fGravity)
{
    m_fGravity = fGravity;
}

void CBullet::Animate(float fElapsedTime)
{
    if (!IsActive()) return;

    XMFLOAT3 xmf3OldPosition = GetPosition();

    XMFLOAT3 xmf3MoveDirection = m_xmf3Direction;

    XMFLOAT3 xmf3NewPosition;
    xmf3NewPosition.x = xmf3OldPosition.x + xmf3MoveDirection.x * m_fSpeed * fElapsedTime;
    xmf3NewPosition.y = xmf3OldPosition.y + xmf3MoveDirection.y * m_fSpeed * fElapsedTime + m_fVerticalVelocity * fElapsedTime;
    xmf3NewPosition.z = xmf3OldPosition.z + xmf3MoveDirection.z * m_fSpeed * fElapsedTime;

    SetPosition(xmf3NewPosition);

    // 중력 적용
    m_fVerticalVelocity += m_fGravity * fElapsedTime;

    float dx = xmf3NewPosition.x - xmf3OldPosition.x;
    float dy = xmf3NewPosition.y - xmf3OldPosition.y;
    float dz = xmf3NewPosition.z - xmf3OldPosition.z;

    m_fTraveledDistance += sqrtf(dx * dx + dy * dy + dz * dz);

    if (m_fTraveledDistance >= m_fRange)
    {
        SetActive(false);
    }
}