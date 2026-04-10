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

void CBullet::Animate(float fElapsedTime)
{
    XMFLOAT3 xmf3Shift;
    XMStoreFloat3(&xmf3Shift,
        XMVectorScale(XMLoadFloat3(&m_xmf3Direction), m_fSpeed * fElapsedTime));

    Move(xmf3Shift, 1.0f);

    m_fTraveledDistance += m_fSpeed * fElapsedTime;

    if (m_fTraveledDistance >= m_fRange)
        m_bActive = false;
}