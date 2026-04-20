// Fragment.cpp
#include "stdafx.h"
#include "GameObject.h"
#include "Fragment.h"

void CFragment::Animate(float fElapsedTime)
{
	CGameObject::Animate(fElapsedTime);

	// 매 프레임 수명 감소
	m_fLifeTime -= fElapsedTime;

	if (m_fLifeTime <= 0.0f)
	{
		m_bActive = false;
	}
}