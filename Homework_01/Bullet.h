// Bullet.h
#pragma once

class CBullet : public CGameObject
{
public:
    CBullet();
    virtual ~CBullet();

    void SetDirection(const XMFLOAT3& xmf3Direction);
    void SetSpeed(float fSpeed);
    void SetRange(float fRange);

    virtual void Animate(float fElapsedTime) override;

private:
    XMFLOAT3 m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
    float m_fSpeed = 30.0f;
    float m_fTraveledDistance = 0.0f;
    float m_fRange = 100.0f;
};

