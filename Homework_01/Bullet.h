// Bullet.h
#pragma once

// 누구의 총알인지 구분
enum class BulletOwner
{
    Player,
    Enemy
};

class CBullet : public CGameObject
{
public:
    CBullet();
    virtual ~CBullet();

    void SetDirection(const XMFLOAT3& xmf3Direction);
    void SetSpeed(float fSpeed);
    void SetRange(float fRange);

    // 포물선용
    void SetVerticalVelocity(float fVelocity);
    void SetGravity(float fGravity);

    virtual void Animate(float fElapsedTime) override;

    void SetBulletOwner(BulletOwner owner) { m_BulletOwner = owner; }
    BulletOwner GetBulletOwner() const { return m_BulletOwner; }

private:
    XMFLOAT3 m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
    float m_fSpeed = 30.0f;
    float m_fTraveledDistance = 0.0f;
    float m_fRange = 100.0f;

    // 포물선용
    float m_fVerticalVelocity = 0.0f;   // 수직 속도
    float m_fGravity = -20.0f;          // 중력 가속도

	BulletOwner m_BulletOwner = BulletOwner::Player;   // 총알 주인 구분
};

