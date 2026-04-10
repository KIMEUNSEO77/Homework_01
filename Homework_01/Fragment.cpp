// Fragment.cpp
#include "stdafx.h"
#include "GameObject.h"
#include "Fragment.h"

void CFragment::Animate(float fElapsedTime)
{
	// 기존 GameObject의 회전/이동 처리 사용
	CGameObject::Animate(fElapsedTime);

	// 수명 감소
	m_fLifeTime -= fElapsedTime;

	if (m_fLifeTime <= 0.0f)
	{
		m_bActive = false;
	}
}