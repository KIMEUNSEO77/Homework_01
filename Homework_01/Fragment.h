// Fragment.h 파편 클래스
#pragma once


class CFragment : public CGameObject
{
public:
	CFragment() {}
	virtual ~CFragment() {}

public:
	float m_fLifeTime = 1.0f;

public:
	void SetLifeTime(float fLifeTime) { m_fLifeTime = fLifeTime; }

	virtual void Animate(float fElapsedTime) override;
};

