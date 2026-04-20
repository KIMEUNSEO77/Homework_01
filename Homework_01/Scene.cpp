// Scene.cpp

#include <random>

#include "stdafx.h"
#include "Scene.h"
#include "GraphicsPipeline.h"

#include <print>

// 축 그리는 함수 (2D)
void DrawLine2D(HDC hDCFrameBuffer, int x0, int y0, int x1, int y1, COLORREF color)
{
	HPEN hPen = ::CreatePen(PS_SOLID, 0, color);
	HPEN hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);

	::MoveToEx(hDCFrameBuffer, x0, y0, NULL);
	::LineTo(hDCFrameBuffer, x1, y1);

	::SelectObject(hDCFrameBuffer, hOldPen);
	::DeleteObject(hPen);
}

void DrawScreenAxis(HDC hDCFrameBuffer, const XMFLOAT4X4& world, int centerX, int centerY, float length)
{
	XMFLOAT3 right(world._11, world._12, world._13);
	XMFLOAT3 up(-world._31, -world._32, -world._33);    // look를 up처럼 사용
	XMFLOAT3 look(world._21, world._22, world._23);  // up을 look처럼 사용

	auto DrawAxis = [&](const XMFLOAT3& axis, COLORREF color)
		{
			int x = (int)(centerX + axis.x * length);
			int y = (int)(centerY - axis.y * length);

			DrawLine2D(hDCFrameBuffer, centerX, centerY, x, y, color);
		};

	DrawAxis(right, RGB(255, 0, 0));   // X
	DrawAxis(up, RGB(0, 255, 0));   // Y
	DrawAxis(look, RGB(0, 0, 255));   // Z
}

// 원 그리는 함수 (2D)
void DrawCircle2D(HDC hDCFrameBuffer, int cx, int cy, int radius, COLORREF color)
{
	HPEN hPen = ::CreatePen(PS_SOLID, 1, color);
	HPEN hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);

	HBRUSH hBrush = (HBRUSH)::GetStockObject(NULL_BRUSH);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hDCFrameBuffer, hBrush);

	::Ellipse(hDCFrameBuffer, cx - radius, cy - radius, cx + radius, cy + radius);

	::SelectObject(hDCFrameBuffer, hOldBrush);
	::SelectObject(hDCFrameBuffer, hOldPen);
	::DeleteObject(hPen);
}

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
	m_pCubeMesh = new CCubeMesh(6.0f, 6.0f, 6.0f);

	// 작은 파편용 큐브 메쉬와 총알용 큐브 메쉬 생성
	m_pFragmentMesh = new CCubeMesh(0.8f, 0.8f, 0.8f);
	m_pBulletMesh = new CCubeMesh(2.0f, 2.0f, 2.0f);

	const int nObjects = 5;

	for (int i = 0; i < nObjects; i++)
	{
		CGameObject* pObject = new CGameObject();

		pObject->SetMesh(m_pCubeMesh);
		pObject->SetColor(ColorUtils::GetRandomColor());

		XMFLOAT3 pos = GetRandomPosition(-50.0f, 50.0f, -40.0f, 40.0f, -60.0f, 60.0f);
		pObject->SetPosition(pos);

		XMFLOAT3 rotAxis = GetRandomDirection();
		pObject->SetRotationAxis(rotAxis);
		pObject->SetRotationSpeed(30.0f + float(rand() % 151));   // 30 ~ 180

		XMFLOAT3 moveDir = GetRandomDirection();
		pObject->SetMovingDirection(moveDir);
		pObject->SetMovingSpeed(float(rand() % 3));   // 0 ~ 2

		pObject->SetCollisionRadius(3.5f);

		m_Objects.push_back(pObject);
	}

	m_pCubeMesh->Release();
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
	if (m_bGameOver || m_bGameClear) return;

	// 플레이어 사망 또는 플레이 시간 초과 체크
	if ((m_pPlayer && m_pPlayer->IsDead()) || m_fPlayTime >= 200.0f)
	{
		GameOver();
		return;
	}

	// 점수 달성 체크
	if (!m_bGameClear && m_nScore >= 1000)
	{
		GameClear();
		return;
	}

	for (CGameObject* pObject : m_Objects)
	{
		if (pObject && pObject->IsActive())
			pObject->Animate(fElapsedTime);
	}

	// Enemy 발사 처리
	for (CGameObject* pObject : m_Objects)
	{
		if (!pObject || !pObject->IsActive()) continue;

		CEnemyObject* pEnemy = dynamic_cast<CEnemyObject*>(pObject);
		if (!pEnemy) continue;

		if (pEnemy->ShouldFireBullet())
		{
			XMFLOAT3 xmf3Position = pEnemy->GetMuzzleWorldPosition();
			XMFLOAT3 xmf3Direction = pEnemy->GetLookVector();

			CreateBullet(xmf3Position, xmf3Direction, RGB(255, 0, 0), BulletOwner::Enemy);

			pEnemy->ResetFireBullet();
		}
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

	// 큐브 스폰 타이머
	m_fObjectSpawnElapsed += fElapsedTime;

	while (m_fObjectSpawnElapsed >= m_fObjectSpawnInterval)
	{
		CreateObject();
		m_fObjectSpawnElapsed -= m_fObjectSpawnInterval;
	}

	// 현재 조준된 타겟이 비활성화되었는지 체크
	if (m_pFocusedTarget && !m_pFocusedTarget->IsActive())
	{
		m_pFocusedTarget = nullptr;
	}

	// 현재 시간
	m_fPlayTime += fElapsedTime;
	// 플래쉬 효과 타이머
	if (m_fFlashTime > 0.0f)
	{
		m_fFlashTime -= fElapsedTime;
		if (m_fFlashTime < 0.0f)
			m_fFlashTime = 0.0f;
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

	// 화면 고정 축
	if (m_pPlayer)
	{
		DrawScreenAxis(hDCFrameBuffer, m_pPlayer->m_xmf4x4World, 80, 80, 40.0f);
	}

	// 플래쉬 효과 렌더링
	RenderDamageFlash(hDCFrameBuffer, 1280, 960);
}

// 총알 생성
void CScene::CreateBullet(const XMFLOAT3& xmf3Position, 
	const XMFLOAT3& xmf3Direction,
	DWORD dwColor, BulletOwner owner)
{
	CBullet* pBullet = new CBullet();
	pBullet->SetMesh(m_pBulletMesh);
	pBullet->SetColor(dwColor);
	pBullet->SetPosition(xmf3Position);
	pBullet->SetDirection(xmf3Direction);
	pBullet->SetSpeed(80.0f);
	pBullet->SetRange(250.0f);

	pBullet->SetVerticalVelocity(10.0f); // 처음에 위로 살짝 뜨기
	pBullet->SetGravity(-20.0f);         // 중력

	pBullet->SetCollisionRadius(0.5f);  // 총알 반지름 설정

	pBullet->SetBulletOwner(owner);

	m_Bullets.push_back(pBullet);
}

// 충돌 체크
void CScene::CheckBulletCollisions()
{
	for (CBullet* pBullet : m_Bullets)
	{
		if (!pBullet || !pBullet->IsActive()) continue;

		BoundingSphere bulletSphere = pBullet->GetBoundingSphere();

		// 적 총알
		if (pBullet->GetBulletOwner() == BulletOwner::Enemy)
		{
			// 플레이어 충돌 검사
			if (m_pPlayer && m_pPlayer->IsActive())
			{
				BoundingOrientedBox playerBox = m_pPlayer->GetBoundingBox();

				if (bulletSphere.Intersects(playerBox))
				{
					m_pPlayer->TakeDamage(10);

					m_fFlashTime = m_fFlashDuration;  // 플래쉬 효과

					pBullet->SetActive(false);
					continue;
				}
			}

			// 일반 오브젝트 충돌 검사
			for (CGameObject* pTarget : m_Objects)
			{
				if (!pTarget || !pTarget->IsActive()) continue;

				// 적은 무시
				CEnemyObject* pEnemy = dynamic_cast<CEnemyObject*>(pTarget);
				if (pEnemy) continue;

				BoundingSphere targetSphere = pTarget->GetBoundingSphere();

				if (bulletSphere.Intersects(targetSphere))
				{
					CreateFragments(pTarget->GetPosition(), pTarget->m_dwColor);

					pBullet->SetActive(false);
					pTarget->SetActive(false);
					break;
				}
			}
			continue;
		}

		// 플레이어 총알
		if (pBullet->GetBulletOwner() == BulletOwner::Player)
		{
			for (CGameObject* pTarget : m_Objects)
			{
				if (!pTarget || !pTarget->IsActive()) continue;

				BoundingSphere targetSphere = pTarget->GetBoundingSphere();

				if (bulletSphere.Intersects(targetSphere))
				{
					CreateFragments(pTarget->GetPosition(), pTarget->m_dwColor);

					pBullet->SetActive(false);
					pTarget->SetActive(false);

					CEnemyObject* pEnemy = dynamic_cast<CEnemyObject*>(pTarget);
					if (pEnemy) AddScore(50);
					else AddScore(10);

					break;
				}
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

	pEnemy->SetMesh(m_pCubeMesh);
	pEnemy->SetColor(ColorUtils::GetRandomColor());

	// 랜덤 위치 생성
	XMFLOAT3 pos = GetRandomPosition(-60.0f, 60.0f, -60.0f, 60.0f, -60.0f, 60.0f);
	pEnemy->SetPosition(pos);

	pEnemy->SetCollisionRadius(3.5f);

	// 플레이어 연결 (추적용)
	pEnemy->SetPlayer(m_pPlayer);

	// 공격 시간 설정 
	float interval = 2.0f + (rand() % 401) / 100.0f; // 2.0 ~ 6.0
	pEnemy->SetAttackInterval(interval);

	// 공격 타이머 설정
	float startOffset = (rand() % 600) / 100.0f; // 0 ~ 6초
	pEnemy->SetAttackElapsed(startOffset);

	m_Objects.push_back(pEnemy);
}

// 게임 오버 처리
void CScene::GameOver()
{
	m_bGameOver = true;

	for (CGameObject* pObject : m_Objects)
		if (pObject) pObject->SetActive(false);
	
	for (CBullet* pBullet : m_Bullets)
		if (pBullet) pBullet->SetActive(false);
	
	for (CFragment* pFragment : m_Fragments)
		if (pFragment) pFragment->SetActive(false);
	
	if (m_pPlayer) m_pPlayer->SetActive(false);

	m_pFocusedTarget = nullptr;
}

// 게임 클리어 처리
void CScene::GameClear()
{
	m_bGameClear = true;

	for (CGameObject* pObject : m_Objects)
		if (pObject) pObject->SetActive(false);

	for (CBullet* pBullet : m_Bullets)
		if (pBullet) pBullet->SetActive(false);

	for (CFragment* pFragment : m_Fragments)
		if (pFragment) pFragment->SetActive(false);

	if (m_pPlayer) m_pPlayer->SetActive(false);

	m_pFocusedTarget = nullptr;
}

void CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_RBUTTONDOWN:
	{
		FocusTargetByMouse(LOWORD(lParam), HIWORD(lParam));
		break;
	}
	case WM_RBUTTONUP:
	{
		ClearFocusedTarget();
		break;
	}
	default:
		break;
	}
}

Ray CScene::GeneratePickingRay(int x, int y, CCamera* pCamera)
{
	Ray ray{};

	if (!pCamera) return ray;

	float viewportWidth = static_cast<float>(pCamera->m_Viewport.m_nWidth);
	float viewportHeight = static_cast<float>(pCamera->m_Viewport.m_nHeight);

	// 화면 좌표 -> NDC
	float ndcX = (2.0f * x / viewportWidth) - 1.0f;
	float ndcY = 1.0f - (2.0f * y / viewportHeight);

	// 투영행렬 역적용
	XMFLOAT3 rayDirView;
	rayDirView.x = ndcX / pCamera->m_xmf4x4Project._11;
	rayDirView.y = ndcY / pCamera->m_xmf4x4Project._22;
	rayDirView.z = 1.0f;
	rayDirView = Vector3Normalize(rayDirView);

	XMFLOAT3 right = pCamera->GetRight();
	XMFLOAT3 up = pCamera->GetUp();
	XMFLOAT3 look = pCamera->GetLook();

	XMFLOAT3 rayDirWorld = Vector3Add(
		Vector3Add(
			Vector3Scale(right, rayDirView.x),
			Vector3Scale(up, rayDirView.y)
		),
		Vector3Scale(look, rayDirView.z)
	);
	rayDirWorld = Vector3Normalize(rayDirWorld);

	ray.origin = pCamera->GetPosition();
	ray.direction = rayDirWorld;

	return ray;
}

bool CScene::IntersectRaySphere(const Ray& ray, const BoundingSphere& sphere, float& fHitDistance)
{
	XMFLOAT3 toCenter = Vector3Subtract(ray.origin, sphere.Center);

	float a = Vector3Dot(ray.direction, ray.direction);
	float b = 2.0f * Vector3Dot(toCenter, ray.direction);
	float c = Vector3Dot(toCenter, toCenter) - (sphere.Radius * sphere.Radius);

	float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0.0f) return false;

	float sqrtDiscriminant = sqrtf(discriminant);

	float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
	float t2 = (-b + sqrtDiscriminant) / (2.0f * a);

	if (t1 >= 0.0f)
		fHitDistance = t1;
	else if (t2 >= 0.0f)
		fHitDistance = t2;
	else
		return false;

	return true;
}

CGameObject* CScene::PickObjectByRay(const Ray& ray, float* pfHitDistance)
{
	CGameObject* pPickedObject = nullptr;
	float fNearestDistance = FLT_MAX;

	for (CGameObject* pObject : m_Objects)
	{
		if (!pObject || !pObject->IsActive()) continue;

		float fDistance = 0.0f;
		if (IntersectRaySphere(ray, pObject->GetBoundingSphere(), fDistance))
		{
			if (fDistance < fNearestDistance)
			{
				fNearestDistance = fDistance;
				pPickedObject = pObject;
			}
		}
	}

	if (pfHitDistance) *pfHitDistance = fNearestDistance;
	return pPickedObject;
}

void CScene::FireBulletToTarget(CGameObject* pTarget)
{
	if (!m_pPlayer || !pTarget) return;

	XMFLOAT3 firePosition = m_pPlayer->GetPosition();
	XMFLOAT3 targetPosition = pTarget->GetPosition();

	XMFLOAT3 fireDirection = Vector3Subtract(targetPosition, firePosition);
	fireDirection = Vector3Normalize(fireDirection);

	CreateBullet(firePosition, fireDirection, RGB(100, 100, 255), BulletOwner::Player);
}

void CScene::FocusTargetByMouse(int x, int y)
{
	if (!m_pPlayer) return;

	CCamera* pCamera = m_pPlayer->GetCamera();
	if (!pCamera) return;

	Ray ray = GeneratePickingRay(x, y, pCamera);

	float fHitDistance = 0.0f;
	m_pFocusedTarget = PickObjectByRay(ray, &fHitDistance);
}

void CScene::ClearFocusedTarget()
{
	m_pFocusedTarget = nullptr;
}

XMFLOAT3 CScene::GetFireDirection() const
{
	if (m_pFocusedTarget && m_pFocusedTarget->IsActive() && m_pPlayer)
	{
		XMFLOAT3 firePosition = m_pPlayer->GetPosition();
		XMFLOAT3 targetPosition = m_pFocusedTarget->GetPosition();

		XMFLOAT3 fireDirection = Vector3Subtract(targetPosition, firePosition);
		return Vector3Normalize(fireDirection);
	}

	// 락온 대상이 없으면 기존처럼 전방 발사
	if (m_pPlayer)
		return m_pPlayer->GetLookVector();

	return XMFLOAT3(0.0f, 0.0f, 1.0f);
}

// 추적용 일반 큐브 생성
void CScene::CreateObject()
{
	CChasingCube* pObject = new CChasingCube();

	pObject->SetMesh(m_pCubeMesh);
	pObject->SetColor(ColorUtils::GetRandomColor());

	XMFLOAT3 pos = GetRandomPosition(-60.0f, 60.0f, -60.0f, 60.0f, -60.0f, 60.0f);
	pObject->SetPosition(pos);

	pObject->SetRotationAxis(GetRandomDirection());
	pObject->SetRotationSpeed(30.0f + float(rand() % 151));

	pObject->SetMovingDirection(GetRandomDirection());
	pObject->SetMovingSpeed(float(rand() % 3));

	pObject->SetCollisionRadius(3.5f);

	pObject->SetPlayer(m_pPlayer);       
	pObject->SetChaseSpeed(2.0f);

	m_Objects.push_back(pObject);
}

// 플래쉬 효과 렌더링
void CScene::RenderDamageFlash(HDC hDCFrameBuffer, int width, int height)
{
	if (m_fFlashTime <= 0.0f) return;

	if (((int)(m_fFlashTime * 30)) % 2 == 0)
	{
		PatBlt(hDCFrameBuffer, 0, 0, width, height, WHITENESS);
	}
}