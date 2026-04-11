// Scene.cpp

#include <random>

#include "stdafx.h"
#include "Scene.h"
#include "GraphicsPipeline.h"


// 랜덤 색상 생성하는 유틸리티 네임스페이스
namespace ColorUtils
{
	COLORREF GetRandomColor()
	{
		// C++11 random 라이브러리 사용 (static으로 두어 매번 생성하지 않도록 함)
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_int_distribution<int> dis(0, 255);

		// R, G, B 각각 0~255 사이의 랜덤 값 반환
		return RGB(dis(gen), dis(gen), dis(gen));
	}
}

void CScene::BuildObjects()
{
	// 직육면체 메쉬를 생성
	CCubeMesh* pCubeMesh = new CCubeMesh(4.0f, 4.0f, 4.0f);

	// 파편용 작은 큐브 메쉬도 생성
	// 파편용 작은 큐브 메쉬
	m_pFragmentMesh = new CCubeMesh(0.5f, 0.5f, 0.5f);

	m_nObjects = 5;
	m_ppObjects = new CGameObject * [m_nObjects];

	m_ppObjects[0] = new CGameObject();
	m_ppObjects[0]->SetMesh(pCubeMesh);
	m_ppObjects[0]->SetColor(ColorUtils::GetRandomColor());
	m_ppObjects[0]->SetPosition(-13.5f, 0.0f, +14.0f);
	m_ppObjects[0]->SetRotationAxis(XMFLOAT3(1.0f, 1.0f, 0.0f));
	m_ppObjects[0]->SetRotationSpeed(90.0f);
	m_ppObjects[0]->SetMovingDirection(XMFLOAT3(1.0f, 0.0f, 0.0f));
	m_ppObjects[0]->SetMovingSpeed(0.5f);
	m_ppObjects[0]->SetCollisionRadius(2.5f);

	m_ppObjects[1] = new CGameObject();
	m_ppObjects[1]->SetMesh(pCubeMesh);
	m_ppObjects[1]->SetColor(ColorUtils::GetRandomColor());
	m_ppObjects[1]->SetPosition(+13.5f, 0.0f, +14.0f);
	m_ppObjects[1]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 1.0f));
	m_ppObjects[1]->SetRotationSpeed(180.0f);
	m_ppObjects[1]->SetMovingDirection(XMFLOAT3(-1.0f, 0.0f, 0.0f));
	m_ppObjects[1]->SetMovingSpeed(1.5f);
	m_ppObjects[1]->SetCollisionRadius(2.5f);

	m_ppObjects[2] = new CGameObject();
	m_ppObjects[2]->SetMesh(pCubeMesh);
	m_ppObjects[2]->SetColor(ColorUtils::GetRandomColor());
	m_ppObjects[2]->SetPosition(0.0f, +5.0f, 20.0f);
	m_ppObjects[2]->SetRotationAxis(XMFLOAT3(1.0f, 0.0f, 1.0f));
	m_ppObjects[2]->SetRotationSpeed(30.15f);
	m_ppObjects[2]->SetMovingDirection(XMFLOAT3(1.0f, -1.0f, 0.0f));
	m_ppObjects[2]->SetMovingSpeed(0.0f);
	m_ppObjects[2]->SetCollisionRadius(2.5f);

	m_ppObjects[3] = new CGameObject();
	m_ppObjects[3]->SetMesh(pCubeMesh);
	m_ppObjects[3]->SetColor(ColorUtils::GetRandomColor());
	m_ppObjects[3]->SetPosition(0.0f, 0.0f, 40.0f);
	m_ppObjects[3]->SetRotationAxis(XMFLOAT3(0.0f, 0.0f, 1.0f));
	m_ppObjects[3]->SetRotationSpeed(40.6f);
	m_ppObjects[3]->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 1.0f));
	m_ppObjects[3]->SetMovingSpeed(0.0f);
	m_ppObjects[3]->SetCollisionRadius(2.5f);

	m_ppObjects[4] = new CGameObject();
	m_ppObjects[4]->SetMesh(pCubeMesh);
	m_ppObjects[4]->SetColor(ColorUtils::GetRandomColor());
	m_ppObjects[4]->SetPosition(10.0f, 10.0f, 50.0f);
	m_ppObjects[4]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 1.0f));
	m_ppObjects[4]->SetRotationSpeed(50.06f);
	m_ppObjects[4]->SetMovingDirection(XMFLOAT3(0.0f, 1.0f, 1.0f));
	m_ppObjects[4]->SetMovingSpeed(0.0f);
	m_ppObjects[4]->SetCollisionRadius(2.5f);
}

void CScene::ReleaseObjects()
{
	for (int i = 0; i < m_nObjects; i++) 
		if (m_ppObjects[i]) delete m_ppObjects[i];

	if (m_ppObjects) delete[] m_ppObjects;

	// 총알도 소멸 처리
	for (CBullet* pBullet : m_Bullets)
		delete pBullet;
	m_Bullets.clear();

	for (CFragment* pFragment : m_Fragments)
		delete pFragment;
	m_Fragments.clear();

	if (m_pFragmentMesh) delete m_pFragmentMesh;
}

void CScene::Animate(float fElapsedTime)
{
	for (int i = 0; i < m_nObjects; i++)
		m_ppObjects[i]->Animate(fElapsedTime);	

	// 총알도 애니메이션 처리
	for (CBullet* pBullet : m_Bullets)
	{
		if (pBullet->IsActive())
			pBullet->Animate(fElapsedTime);
	}

	// 파편도 애니메이션 처리
	for (CFragment* pFragment : m_Fragments)
	{
		if (pFragment && pFragment->IsActive())
			pFragment->Animate(fElapsedTime);
	}

	// 충돌 체크
	CheckBulletCollisions();

	// 죽은 총알 제거
	RemoveDeadBullets();

	// 죽은 파편 제거
	RemoveDeadFragments();
}

void CScene::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGraphicsPipeline::SetViewport(&pCamera->m_Viewport);
	CGraphicsPipeline::SetViewProjectTransform(&pCamera->m_xmf4x4ViewProject);

	for (int i = 0; i < m_nObjects; i++)
	{
		if (m_ppObjects[i] && m_ppObjects[i]->IsActive())
			m_ppObjects[i]->Render(hDCFrameBuffer, pCamera);
	}

	// 총알도 렌더링 처리
	for (CBullet* pBullet : m_Bullets)
	{
		if (pBullet && pBullet->IsActive())
			pBullet->Render(hDCFrameBuffer, pCamera);
	}

	// 파편도 렌더링 처리
	for (CFragment* pFragment : m_Fragments)
	{
		if (pFragment && pFragment->IsActive())
			pFragment->Render(hDCFrameBuffer, pCamera);
	}
}

// 총알 생성
void CScene::CreateBullet(const XMFLOAT3& xmf3Position, const XMFLOAT3& xmf3Direction)
{
	CCubeMesh* pBulletMesh = new CCubeMesh(2.0f, 2.0f, 2.0f);

	CBullet* pBullet = new CBullet();
	pBullet->SetMesh(pBulletMesh);
	pBullet->SetColor(RGB(255, 0, 0));
	pBullet->SetPosition(xmf3Position);
	pBullet->SetDirection(xmf3Direction);
	pBullet->SetSpeed(50.0f);
	pBullet->SetRange(200.0f);

	pBullet->SetCollisionRadius(0.5f);  // 총알 반지름 설정

	m_Bullets.push_back(pBullet);
}

// 충돌 체크
void CScene::CheckBulletCollisions()
{
	for (CBullet* pBullet : m_Bullets)
	{
		if (!pBullet || !pBullet->IsActive()) continue;

		BoundingSphere bulletSphere = pBullet->GetBoundingSphere();

		for (int i = 0; i < m_nObjects; i++)
		{
			CGameObject* pTarget = m_ppObjects[i];
			if (!pTarget || !pTarget->IsActive()) continue;

			BoundingSphere targetSphere = pTarget->GetBoundingSphere();

			if (bulletSphere.Intersects(targetSphere))
			{
				CreateFragments(pTarget->GetPosition(), pBullet->m_dwColor);

				pBullet->SetActive(false);
				pTarget->SetActive(false);
				break;
			}
		}
	}
}

// 죽은 총알 제거
void CScene::RemoveDeadBullets()
{
	m_Bullets.erase(std::remove_if(m_Bullets.begin(), m_Bullets.end(),
		[](CBullet* pBullet) {
			if (!pBullet->IsActive())
			{
				delete pBullet; // 메모리 해제
				return true;    // 제거 대상
			}
			return false;       // 유지 대상
		}), m_Bullets.end());
}

// 랜덤한 방향 벡터 생성
XMFLOAT3 CScene::GetRandomDirection()
{
	float x = float((rand() % 200) - 100);
	float y = float((rand() % 200) - 100);
	float z = float((rand() % 200) - 100);

	XMFLOAT3 xmf3Direction = XMFLOAT3(x, y, z);

	XMVECTOR xmvDirection = XMVector3Normalize(XMLoadFloat3(&xmf3Direction));
	XMStoreFloat3(&xmf3Direction, xmvDirection);

	return xmf3Direction;
}

// 파편 생성
void CScene::CreateFragments(const XMFLOAT3& xmf3Position, DWORD dwColor)
{
	for (int i = 0; i < 8; i++)
	{
		CFragment* pFragment = new CFragment();

		pFragment->SetMesh(m_pFragmentMesh);
		pFragment->SetColor(dwColor);
		pFragment->SetPosition(xmf3Position);

		XMFLOAT3 xmf3Direction = GetRandomDirection();
		pFragment->SetMovingDirection(xmf3Direction);
		pFragment->SetMovingSpeed(5.0f + float(rand() % 6));   // 5 ~ 10

		XMFLOAT3 xmf3RotationAxis = GetRandomDirection();
		pFragment->SetRotationAxis(xmf3RotationAxis);
		pFragment->SetRotationSpeed(180.0f + float(rand() % 181));   // 180 ~ 360

		pFragment->SetLifeTime(1.0f + float(rand() % 100) / 100.0f); // 1.0 ~ 1.99
		pFragment->SetCollisionRadius(0.5f);

		m_Fragments.push_back(pFragment);
	}
}

// 죽은 파편 처리
void CScene::RemoveDeadFragments()
{
	for (auto it = m_Fragments.begin(); it != m_Fragments.end(); )
	{
		if (!(*it)->IsActive())
		{
			delete (*it);
			it = m_Fragments.erase(it);
		}
		else
		{
			++it;
		}
	}
}