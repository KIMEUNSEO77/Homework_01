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

		XMFLOAT3 dir;
		dir.x = playerPos.x - myPos.x;
		dir.y = playerPos.y - myPos.y;
		dir.z = playerPos.z - myPos.z;

		SetMovingDirection(dir);
		SetMovingSpeed(2.0f);
	}

	// 공격 타이머 (나중에 사용)
	m_fAttackElapsed += fElapsedTime;
	if (m_fAttackElapsed >= m_fAttackInterval)
	{
		// Attack(); 나중에 구현
		m_fAttackElapsed = 0.0f;
	}
}