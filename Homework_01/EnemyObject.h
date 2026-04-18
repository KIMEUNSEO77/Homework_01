// EnemyObject.h
#pragma once

class CEnemyObject : public CGameObject
{
public:
	CEnemyObject() {}
	virtual ~CEnemyObject() {}

private:
	CPlayer* m_pPlayer = nullptr;

	float m_fAttackElapsed = 0.0f;
	float m_fAttackInterval = 0.0f;

	bool m_bFireBullet = false;

public:
	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	void SetAttackInterval(float fInterval) { m_fAttackInterval = fInterval; }  // 공격 시간 설정
	void SetAttackElapsed(float fElapsed) { m_fAttackElapsed = fElapsed; }  // 공격 타이머 초기화

	virtual void Animate(float fElapsedTime) override;
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera) override;

	// 공격 위치 구하는 함수
	XMFLOAT3 GetMuzzleWorldPosition() const;
	XMFLOAT3 GetLookVector() const;

	bool ShouldFireBullet() const { return m_bFireBullet; }
	void ResetFireBullet() { m_bFireBullet = false; }

	// 플레이어를 바라보도록
	void UpdateLookAtPlayer();
};

