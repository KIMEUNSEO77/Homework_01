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
	float m_fAttackInterval = 2.0f;

public:
	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }

	virtual void Animate(float fElapsedTime) override;
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera) override;

	// 공격 위치 구하는 함수
	XMFLOAT3 GetMuzzleWorldPosition() const;
};

