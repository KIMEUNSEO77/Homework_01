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

// 랜덤 위치 만들기
XMFLOAT3 CScene::GetRandomPosition(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
	float x = minX + static_cast<float>(rand()) / RAND_MAX * (maxX - minX);
	float y = minY + static_cast<float>(rand()) / RAND_MAX * (maxY - minY);
	float z = minZ + static_cast<float>(rand()) / RAND_MAX * (maxZ - minZ);

	return XMFLOAT3(x, y, z);
}

void CScene::BuildObjects()
{
	// 직육면체 메쉬를 생성
	CCubeMesh* pCubeMesh = new CCubeMesh(4.0f, 4.0f, 4.0f);

	// 파편용 작은 큐브 메쉬도 생성
	m_pFragmentMesh = new CCubeMesh(0.5f, 0.5f, 0.5f);

	// 적 객체용 큐브 메쉬도 생성
	m_pEnemyMesh = new CCubeMesh(4.0f, 4.0f, 4.0f);

	const int nObjects = 5;

	for (int i = 0; i < nObjects; i++)
	{
		CGameObject* pObject = new CGameObject();

		pObject->SetMesh(pCubeMesh);
		pObject->SetColor(ColorUtils::GetRandomColor());

		XMFLOAT3 pos = GetRandomPosition(-50.0f, 50.0f, -10.0f, 10.0f, 40.0f, 60.0f);
		pObject->SetPosition(pos);

		XMFLOAT3 rotAxis = GetRandomDirection();
		pObject->SetRotationAxis(rotAxis);
		pObject->SetRotationSpeed(30.0f + float(rand() % 151));   // 30 ~ 180

		XMFLOAT3 moveDir = GetRandomDirection();
		pObject->SetMovingDirection(moveDir);
		pObject->SetMovingSpeed(float(rand() % 3));   // 0 ~ 2

		pObject->SetCollisionRadius(2.5f);

		m_Objects.push_back(pObject);
	}

	pCubeMesh->Release();
}

void CScene::ReleaseObjects()
{
	for (CGameObject* pObject : m_Objects)
		delete pObject;
	m_Objects.clear();

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
	for (CGameObject* pObject : m_Objects)
	{
		if (pObject && pObject->IsActive())
			pObject->Animate(fElapsedTime);
	}

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

	// 적 스폰 타이머
	m_fEnemySpawnElapsed += fElapsedTime;
	// 적 생성
	while (m_fEnemySpawnElapsed >= m_fEnemySpawnInterval)
	{
		CreateEnemy();
		m_fEnemySpawnElapsed -= m_fEnemySpawnInterval;
	}
}

void CScene::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGraphicsPipeline::SetViewport(&pCamera->m_Viewport);
	CGraphicsPipeline::SetViewProjectTransform(&pCamera->m_xmf4x4ViewProject);

	for (CGameObject* pObject : m_Objects)
	{
		if (pObject && pObject->IsActive())
			pObject->Render(hDCFrameBuffer, pCamera);
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

	pBullet->SetVerticalVelocity(10.0f); // 처음에 위로 살짝 뜨기
	pBullet->SetGravity(-20.0f);         // 중력

	pBullet->SetCollisionRadius(0.5f);  // 총알 반지름 설정

	m_Bullets.push_back(pBullet);

	pBulletMesh->Release();
}

// 충돌 체크
void CScene::CheckBulletCollisions()
{
	for (CBullet* pBullet : m_Bullets)
	{
		if (!pBullet || !pBullet->IsActive()) continue;

		BoundingSphere bulletSphere = pBullet->GetBoundingSphere();

		for (CGameObject* pTarget : m_Objects)
		{
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

// 적 생성
void CScene::CreateEnemy()
{
	CEnemyObject* pEnemy = new CEnemyObject();

	pEnemy->SetMesh(m_pEnemyMesh);
	pEnemy->SetColor(ColorUtils::GetRandomColor());

	// 랜덤 위치 생성
	XMFLOAT3 pos = GetRandomPosition(-40.0f, 40.0f, -20.0f, 20.0f, 50.0f, 70.0f);
	pEnemy->SetPosition(pos);

	pEnemy->SetRotationAxis(GetRandomDirection());
	pEnemy->SetRotationSpeed(0.0f);

	pEnemy->SetCollisionRadius(2.5f);

	// 플레이어 연결 (추적용)
	pEnemy->SetPlayer(m_pPlayer);

	m_Objects.push_back(pEnemy);
}