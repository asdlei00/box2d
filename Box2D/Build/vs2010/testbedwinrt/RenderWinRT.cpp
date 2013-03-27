#include "pch.h"
#include "RenderWinRT.h"
#include "CommonStates.h"
#include "Effects.h"
#include "PrimitiveBatch.h"
#include "VertexTypes.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include <d3d11.h>
#include "CubeRenderer.h"
#include "TestbedWinRT.h"

using namespace DirectX;

DebugDraw::DebugDraw()
{
	CubeRenderer ^cubeRenderer = CubeRenderer::GetInstance();
	m_commonStates = cubeRenderer->GetCommonStates();
	m_basicEffect = cubeRenderer->GetBasicEffect();
	m_renderer = cubeRenderer->GetBatchDrawer();
	m_spriteBatch = cubeRenderer->GetSpriteBatch();
	m_spriteFont = cubeRenderer->GetSpriteFont();
	m_d3dContext = cubeRenderer->GetDeviceContext();
}

void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	const int32 totalVertexCount = vertexCount + 1;
	const b2Vec2 *currentVertex = vertices;
	VertexPositionColor *processedVertices = new VertexPositionColor[totalVertexCount];
	VertexPositionColor *currentProcessedVertex = processedVertices;
	for(int32 i = 0; i < vertexCount; ++i)
	{
		currentProcessedVertex->color.x = color.r;
		currentProcessedVertex->color.y = color.g;
		currentProcessedVertex->color.z = color.b;
		currentProcessedVertex->color.w = 1;
		currentProcessedVertex->position.x = currentVertex->x;
		currentProcessedVertex->position.y = currentVertex->y;
		currentProcessedVertex->position.z = 0;
		++currentVertex;
		++currentProcessedVertex;
	}
	currentProcessedVertex->color.x = color.r;
	currentProcessedVertex->color.y = color.g;
	currentProcessedVertex->color.z = color.b;
	currentProcessedVertex->color.w = 1;
	currentProcessedVertex->position.x = vertices->x;
	currentProcessedVertex->position.y = vertices->y;
	currentProcessedVertex->position.z = 0;
	m_renderer->Begin();
	m_renderer->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, processedVertices, totalVertexCount);
	m_renderer->End();
	delete [] processedVertices;
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	m_d3dContext->OMSetBlendState(m_commonStates->AlphaBlend(), NULL, 0xffffffff);
	m_renderer->Begin();
	VertexPositionColor firstProcessedVertex(XMFLOAT3(vertices->x, vertices->y, 0), XMFLOAT4(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, 0.5f));
	const b2Vec2 *currentVertex = vertices + 1;
	for(int32 i = 2; i < vertexCount; ++i)
	{
		VertexPositionColor processedVertex1(XMFLOAT3(currentVertex->x, currentVertex->y, 0), XMFLOAT4(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, 0.5f));
		++currentVertex;
		VertexPositionColor processedVertex2(XMFLOAT3(currentVertex->x, currentVertex->y, 0), XMFLOAT4(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, 0.5f));
		m_renderer->DrawTriangle(firstProcessedVertex, processedVertex1, processedVertex2);
	}
	m_renderer->End();

	m_d3dContext->OMSetBlendState(m_commonStates->Opaque(), NULL, 0xffffffff);
	m_basicEffect->Apply(m_d3dContext);
	DrawPolygon(vertices, vertexCount, color);
}

void DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
	m_renderer->Begin();
	for(int32 i = 0; i < 16; ++i)
	{
		float theta = b2_pi / 8.0f * i;
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		VertexPositionColor processedVertex0(XMFLOAT3(v.x, v.y, 0), XMFLOAT4(color.r, color.g, color.b, 1));
		theta = b2_pi / 8.0f * (i + 1);
		v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		VertexPositionColor processedVertex1(XMFLOAT3(v.x, v.y, 0), XMFLOAT4(color.r, color.g, color.b, 1));
		m_renderer->DrawLine(processedVertex0, processedVertex1);
	}
	m_renderer->End();
}

void DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
	m_d3dContext->OMSetBlendState(m_commonStates->AlphaBlend(), NULL, 0xffffffff);
	VertexPositionColor processedVertex0(XMFLOAT3(center.x, center.y, 0), XMFLOAT4(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, 0.5f));
	m_renderer->Begin();
	for(int32 i = 0; i < 16; ++i)
	{
		float theta = b2_pi / 8.0f * i;
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		VertexPositionColor processedVertex1(XMFLOAT3(v.x, v.y, 0), XMFLOAT4(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, 0.5f));
		theta = b2_pi / 8.0f * (i + 1);
		v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		VertexPositionColor processedVertex2(XMFLOAT3(v.x, v.y, 0), XMFLOAT4(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, 0.5f));
		m_renderer->DrawTriangle(processedVertex0, processedVertex1, processedVertex2);
	}
	m_renderer->End();
	
	m_d3dContext->OMSetBlendState(m_commonStates->Opaque(), NULL, 0xffffffff);
	m_basicEffect->Apply(m_d3dContext);
	DrawCircle(center, radius, color);

	b2Vec2 p = center + radius * axis;
	DrawSegment(center, p, color);
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	m_renderer->Begin();
	m_renderer->DrawLine(
		VertexPositionColor(XMFLOAT3(p1.x, p1.y, 0), XMFLOAT4(color.r, color.g, color.b, 1)),
		VertexPositionColor(XMFLOAT3(p2.x, p2.y, 0), XMFLOAT4(color.r, color.g, color.b, 1))
	);
	m_renderer->End();
}

void DebugDraw::DrawTransform(const b2Transform& xf)
{
	b2Vec2 p1 = xf.p, p2;
	const float32 k_axisScale = 10.0f;
	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	m_renderer->Begin();
	m_renderer->DrawLine(
		VertexPositionColor(XMFLOAT3(p1.x, p1.y, 0), XMFLOAT4(1, 0, 0, 1)),
		VertexPositionColor(XMFLOAT3(p2.x, p2.y, 0), XMFLOAT4(1, 0, 0, 1))
	);
	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	m_renderer->DrawLine(
		VertexPositionColor(XMFLOAT3(p1.x, p1.y, 0), XMFLOAT4(0, 1, 0, 1)),
		VertexPositionColor(XMFLOAT3(p2.x, p2.y, 0), XMFLOAT4(0, 1, 0, 1))
	);
	m_renderer->End();
	//b2Vec2 p1 = xf.p, p2;
	//const float32 k_axisScale = 0.4f;
	//glBegin(GL_LINES);
	//
	//glColor3f(1.0f, 0.0f, 0.0f);
	//glVertex2f(p1.x, p1.y);
	//p2 = p1 + k_axisScale * xf.q.GetXAxis();
	//glVertex2f(p2.x, p2.y);

	//glColor3f(0.0f, 1.0f, 0.0f);
	//glVertex2f(p1.x, p1.y);
	//p2 = p1 + k_axisScale * xf.q.GetYAxis();
	//glVertex2f(p2.x, p2.y);

	//glEnd();
}

void DebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
{
	float halfSize = size * 0.5f;
	XMFLOAT4 processedColor(color.r, color.g, color.b, 1);
	XMFLOAT3 lowerLeft(p.x - halfSize, p.y - halfSize, 0);
	XMFLOAT3 upperLeft(p.x - halfSize, p.y + halfSize, 0);
	XMFLOAT3 lowerRight(p.x + halfSize, p.y - halfSize, 0);
	XMFLOAT3 upperRight(p.x + halfSize, p.y + halfSize, 0);
	m_renderer->Begin();
	m_renderer->DrawQuad(
		VertexPositionColor(lowerLeft, processedColor),
		VertexPositionColor(lowerRight, processedColor),
		VertexPositionColor(upperRight, processedColor),
		VertexPositionColor(upperLeft, processedColor)
	);
	m_renderer->End();
}

void DebugDraw::DrawString(int x, int y, const char *string, ...)
{
	char buffer[128];

	va_list arg;
	va_start(arg, string);
	vsprintf_s(buffer, string, arg);
	va_end(arg);

	UINT viewportCount = 1;
	D3D11_VIEWPORT viewport;
	m_d3dContext->RSGetViewports(&viewportCount, &viewport);
	m_basicEffect->SetProjection(XMMatrixOrthographicOffCenterRH(0, viewport.Width, viewport.Height, 0, -1, 1));
	m_basicEffect->SetView(XMMatrixIdentity());
	m_basicEffect->SetWorld(XMMatrixIdentity());

	wchar_t wbuffer[128];
	size_t printed;
	mbstowcs_s(&printed, wbuffer, buffer, sizeof(wbuffer));

	m_spriteBatch->Begin();
	m_spriteFont->DrawString(m_spriteBatch, wbuffer, XMFLOAT2((float)x, (float)y), XMLoadFloat4(&XMFLOAT4(0.9f, 0.6f, 0.6f, 1.0f)));
	m_spriteBatch->End();

	TestbedWinRT::Resize(0, 0);
	m_d3dContext->RSSetState(m_commonStates->CullClockwise());

	//glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	//glLoadIdentity();
	//int w = glutGet(GLUT_WINDOW_WIDTH);
	//int h = glutGet(GLUT_WINDOW_HEIGHT);
	//gluOrtho2D(0, w, h, 0);
	//glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	//glLoadIdentity();

	//glColor3f(0.9f, 0.6f, 0.6f);
	//glRasterPos2i(x, y);
	//int32 length = (int32)strlen(buffer);
	//for (int32 i = 0; i < length; ++i)
	//{
	//	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
	//	//glutBitmapCharacter(GLUT_BITMAP_9_BY_15, buffer[i]);
	//}

	//glPopMatrix();
	//glMatrixMode(GL_PROJECTION);
	//glPopMatrix();
	//glMatrixMode(GL_MODELVIEW);
}

void DebugDraw::DrawString(const b2Vec2& p, const char *string, ...)
{
	char buffer[128];

	va_list arg;
	va_start(arg, string);
	vsprintf_s(buffer, string, arg);
	va_end(arg);

	wchar_t wbuffer[128];
	size_t printed;
	mbstowcs_s(&printed, wbuffer, buffer, sizeof(wbuffer));

	m_spriteBatch->Begin();
	m_spriteFont->DrawString(m_spriteBatch, wbuffer, XMFLOAT2(p.x, p.y), XMLoadFloat4(&XMFLOAT4(0.5f, 0.9f, 0.5f, 1.0f)));
	m_spriteBatch->End();
	
	m_d3dContext->RSSetState(m_commonStates->CullClockwise());

	//glColor3f(0.5f, 0.9f, 0.5f);
	//glRasterPos2f(p.x, p.y);

	//int32 length = (int32)strlen(buffer);
	//for (int32 i = 0; i < length; ++i)
	//{
	//	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, buffer[i]);
	//}

	//glPopMatrix();
}

void DebugDraw::DrawAABB(b2AABB* aabb, const b2Color& c)
{
	b2Vec2 center(aabb->GetCenter());
	b2Vec2 extents(aabb->GetExtents());
	XMFLOAT4 processedColor(c.r, c.g, c.b, 1);
	XMFLOAT3 lowerLeft(center.x - extents.x, center.y - extents.y, 0);
	XMFLOAT3 upperLeft(center.x - extents.x, center.y + extents.y, 0);
	XMFLOAT3 lowerRight(center.x + extents.x, center.y - extents.y, 0);
	XMFLOAT3 upperRight(center.x + extents.x, center.y + extents.y, 0);
	VertexPositionColor vertices[5] = {
		VertexPositionColor(lowerLeft, processedColor),
		VertexPositionColor(lowerRight, processedColor),
		VertexPositionColor(upperRight, processedColor),
		VertexPositionColor(upperLeft, processedColor),
		VertexPositionColor(lowerLeft, processedColor)
	};
	m_renderer->Begin();
	m_renderer->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, vertices, 5);
	m_renderer->End();
}
