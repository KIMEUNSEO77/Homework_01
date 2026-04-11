// EnemyObject.cpp
#include "stdafx.h"
#include "GameObject.h"
#include "Player.h"
#include "EnemyObject.h"

void CEnemyObject::Animate(float fElapsedTime)
{
	// 기본 이동/회전 처리
	CGameObject::Animate(fElapsedTime);

	// 플레이어 추적 (간단 버전)
	if (m_pPlayer)
	{
		XMFLOAT3 playerPos = m_pPlayer->GetPosition();
		XMFLOAT3 myPos = GetPosition();

		XMFLOAT3 dir = XMFLOAT3(
			playerPos.x - myPos.x,
			playerPos.y - myPos.y,
			playerPos.z - myPos.z
		);

		SetMovingDirection(dir);
		SetMovingSpeed(2.0f);
	}

	// 공격 타이머 
	m_fAttackElapsed += fElapsedTime;
	if (m_fAttackElapsed >= m_fAttackInterval)
	{
		// Attack(); 나중에 구현
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