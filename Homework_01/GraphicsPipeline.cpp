// GraphicsPipeline.cpp

#include "stdafx.h"
#include "GraphicsPipeline.h"

XMFLOAT4X4* CGraphicsPipeline::m_pxmf4x4World = NULL;
XMFLOAT4X4* CGraphicsPipeline::m_pxmf4x4ViewProject = NULL;
CViewport* CGraphicsPipeline::m_pViewport = NULL;


// 모델 좌표계의 점을 월드 변환, 카메라 변환, 원근 투영 변환을 순차적으로 수행
// 월드 변환은 게임 객체의 정보(위치, 방향) 필요
// 카메라 변환은 카메라의 정보(위치, 방향) 필요
// 원근 투영 변환은 카메라의 정보(시야각, 종횡비) 필요
XMFLOAT3 CGraphicsPipeline::Project(XMFLOAT3& xmf3Model)
{
	XMMATRIX xmmtxModelToProject =
		XMMatrixMultiply(XMLoadFloat4x4(m_pxmf4x4World),
			XMLoadFloat4x4(m_pxmf4x4ViewProject));

	XMFLOAT3 xmf3Project;
	XMStoreFloat3(&xmf3Project,
		XMVector3TransformCoord(XMLoadFloat3(&xmf3Model),
			xmmtxModelToProject));

	return(xmf3Project);
}

// 투영 좌표계의 점을 화면 변환을 수행
// 화면 변환은 카메라의 정보(뷰포트) 필요
XMFLOAT3 CGraphicsPipeline::ScreenTransform(XMFLOAT3& xmf3Project)
{
	XMFLOAT3 f3Screen = xmf3Project;

	float fHalfWidth = m_pViewport->m_nWidth * 0.5f;
	float fHalfHeight = m_pViewport->m_nHeight * 0.5f;

	f3Screen.x = m_pViewport->m_nLeft + (xmf3Project.x * fHalfWidth) +
		fHalfWidth;
	f3Screen.y = m_pViewport->m_nTop + (-xmf3Project.y * fHalfHeight) +
		fHalfHeight;

	return(f3Screen);
}