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

	m_ppObjects[1] = new CGameObject();
	m_ppObjects[1]->SetMesh(pCubeMesh);
	m_ppObjects[1]->SetColor(ColorUtils::GetRandomColor());
	m_ppObjects[1]->SetPosition(+13.5f, 0.0f, +14.0f);
	m_ppObjects[1]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 1.0f));
	m_ppObjects[1]->SetRotationSpeed(180.0f);
	m_ppObjects[1]->SetMovingDirection(XMFLOAT3(-1.0f, 0.0f, 0.0f));
	m_ppObjects[1]->SetMovingSpeed(1.5f);

	m_ppObjects[2] = new CGameObject();
	m_ppObjects[2]->SetMesh(pCubeMesh);
	m_ppObjects[2]->SetColor(ColorUtils::GetRandomColor());
	m_ppObjects[2]->SetPosition(0.0f, +5.0f, 20.0f);
	m_ppObjects[2]->SetRotationAxis(XMFLOAT3(1.0f, 0.0f, 1.0f));
	m_ppObjects[2]->SetRotationSpeed(30.15f);
	m_ppObjects[2]->SetMovingDirection(XMFLOAT3(1.0f, -1.0f, 0.0f));
	m_ppObjects[2]->SetMovingSpeed(0.0f);

	m_ppObjects[3] = new CGameObject();
	m_ppObjects[3]->SetMesh(pCubeMesh);
	m_ppObjects[3]->SetColor(ColorUtils::GetRandomColor());
	m_ppObjects[3]->SetPosition(0.0f, 0.0f, 40.0f);
	m_ppObjects[3]->SetRotationAxis(XMFLOAT3(0.0f, 0.0f, 1.0f));
	m_ppObjects[3]->SetRotationSpeed(40.6f);
	m_ppObjects[3]->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 1.0f));
	m_ppObjects[3]->SetMovingSpeed(0.0f);

	m_ppObjects[4] = new CGameObject();
	m_ppObjects[4]->SetMesh(pCubeMesh);
	m_ppObjects[4]->SetColor(ColorUtils::GetRandomColor());
	m_ppObjects[4]->SetPosition(10.0f, 10.0f, 50.0f);
	m_ppObjects[4]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 1.0f));
	m_ppObjects[4]->SetRotationSpeed(50.06f);
	m_ppObjects[4]->SetMovingDirection(XMFLOAT3(0.0f, 1.0f, 1.0f));
	m_ppObjects[4]->SetMovingSpeed(0.0f);
}

void CScene::ReleaseObjects()
{
	for (int i = 0; i < m_nObjects; i++) 
		if (m_ppObjects[i]) delete m_ppObjects[i];

	if (m_ppObjects) delete[] m_ppObjects;

	// 총알도 소멸 처리
	for (CBullet* pBullet : m_vBullets)
		delete pBullet;
	m_vBullets.clear();
}

void CScene::Animate(float fElapsedTime)
{
	for (int i = 0; i < m_nObjects; i++)
		m_ppObjects[i]->Animate(fElapsedTime);	

	// 총알도 애니메이션 처리
	for (auto it = m_vBullets.begin(); it != m_vBullets.end(); )
	{
		(*it)->Animate(fElapsedTime);

		if (!(*it)->IsActive())
		{
			delete (*it);
			it = m_vBullets.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void CScene::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGraphicsPipeline::SetViewport(&pCamera->m_Viewport);
	CGraphicsPipeline::SetViewProjectTransform(&pCamera->m_xmf4x4ViewProject);

	for (int i = 0; i < m_nObjects; i++)
		m_ppObjects[i]->Render(hDCFrameBuffer, pCamera);

	// 총알도 렌더링 처리
	for (CBullet* pBullet : m_vBullets)
		pBullet->Render(hDCFrameBuffer, pCamera);
}

// 총알 생성
void CScene::CreateBullet(const XMFLOAT3& xmf3Position, const XMFLOAT3& xmf3Direction)
{
	CCubeMesh* pBulletMesh = new CCubeMesh(0.5f, 0.5f, 1.5f);

	CBullet* pBullet = new CBullet();
	pBullet->SetMesh(pBulletMesh);
	pBullet->SetColor(RGB(255, 0, 0));
	pBullet->SetPosition(const_cast<XMFLOAT3&>(xmf3Position));
	pBullet->SetDirection(xmf3Direction);
	pBullet->SetSpeed(50.0f);
	pBullet->SetRange(200.0f);

	m_vBullets.push_back(pBullet);
}