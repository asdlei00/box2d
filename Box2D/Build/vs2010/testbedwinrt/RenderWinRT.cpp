#include "pch.h"
#include "RenderWinRT.h"
#include "CommonStates.h"
#include "PrimitiveBatch.h"
#include "VertexTypes.h"
#include <d3d11.h>

DebugDraw::DebugDraw(DirectX::CommonStates &commonStates, DirectX::PrimitiveBatch<DirectX::VertexPositionColor> &renderer, ID3D11DeviceContext *d3dContext)
: m_commonStates(commonStates), m_renderer(renderer), m_d3dContext(d3dContext)
{
}

void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	const int32 totalVertexCount = vertexCount + 1;
	const b2Vec2 *currentVertex = vertices;
	DirectX::VertexPositionColor *processedVertices = new DirectX::VertexPositionColor[totalVertexCount];
	DirectX::VertexPositionColor *currentProcessedVertex = processedVertices;
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
	m_renderer.Begin();
	m_renderer.Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, processedVertices, totalVertexCount);
	m_renderer.End();
	delete [] processedVertices;
	//glColor3f(color.r, color.g, color.b);
	//glBegin(GL_LINE_LOOP);
	//for (int32 i = 0; i < vertexCount; ++i)
	//{
	//	glVertex2f(vertices[i].x, vertices[i].y);
	//}
	//glEnd();
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	m_d3dContext->OMSetBlendState(m_commonStates.AlphaBlend(), NULL, 0xffffffff);
	m_renderer.Begin();
	DirectX::VertexPositionColor firstProcessedVertex(DirectX::XMFLOAT3(vertices->x, vertices->y, 0), DirectX::XMFLOAT4(color.r, color.g, color.b, 0.5f));
	const b2Vec2 *currentVertex = vertices + 1;
	for(int32 i = 2; i < vertexCount; ++i)
	{
		DirectX::VertexPositionColor processedVertex1(DirectX::XMFLOAT3(currentVertex->x, currentVertex->y, 0), DirectX::XMFLOAT4(color.r, color.g, color.b, 1));
		++currentVertex;
		DirectX::VertexPositionColor processedVertex2(DirectX::XMFLOAT3(currentVertex->x, currentVertex->y, 0), DirectX::XMFLOAT4(color.r, color.g, color.b, 1));
		m_renderer.DrawTriangle(firstProcessedVertex, processedVertex1, processedVertex2);
	}
	m_renderer.End();

	m_d3dContext->OMSetBlendState(m_commonStates.Opaque(), NULL, 0xffffffff);
	DrawPolygon(vertices, vertexCount, color);
	//glEnable(GL_BLEND);
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glColor4f(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);
	//glBegin(GL_TRIANGLE_FAN);
	//for (int32 i = 0; i < vertexCount; ++i)
	//{
	//	glVertex2f(vertices[i].x, vertices[i].y);
	//}
	//glEnd();
	//glDisable(GL_BLEND);

	//glColor4f(color.r, color.g, color.b, 1.0f);
	//glBegin(GL_LINE_LOOP);
	//for (int32 i = 0; i < vertexCount; ++i)
	//{
	//	glVertex2f(vertices[i].x, vertices[i].y);
	//}
	//glEnd();
}

void DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
	m_renderer.Begin();
	for(int32 i = 0; i < 16; ++i)
	{
		float theta = b2_pi / 8.0f * i;
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		DirectX::VertexPositionColor processedVertex0(DirectX::XMFLOAT3(v.x, v.y, 0), DirectX::XMFLOAT4(color.r, color.g, color.b, 1));
		theta = b2_pi / 8.0f * (i + 1);
		v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		DirectX::VertexPositionColor processedVertex1(DirectX::XMFLOAT3(v.x, v.y, 0), DirectX::XMFLOAT4(color.r, color.g, color.b, 1));
		m_renderer.DrawLine(processedVertex0, processedVertex1);
	}
	m_renderer.End();
	//const float32 k_segments = 16.0f;
	//const float32 k_increment = 2.0f * b2_pi / k_segments;
	//float32 theta = 0.0f;
	//glColor3f(color.r, color.g, color.b);
	//glBegin(GL_LINE_LOOP);
	//for (int32 i = 0; i < k_segments; ++i)
	//{
	//	b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
	//	glVertex2f(v.x, v.y);
	//	theta += k_increment;
	//}
	//glEnd();
}

void DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
	m_d3dContext->OMSetBlendState(m_commonStates.AlphaBlend(), NULL, 0xffffffff);
	DirectX::VertexPositionColor processedVertex0(DirectX::XMFLOAT3(center.x, center.y, 0), DirectX::XMFLOAT4(color.r, color.g, color.b, 0.0f));
	m_renderer.Begin();
	for(int32 i = 0; i < 16; ++i)
	{
		float theta = b2_pi / 8.0f * i;
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		DirectX::VertexPositionColor processedVertex1(DirectX::XMFLOAT3(v.x, v.y, 0), DirectX::XMFLOAT4(color.r, color.g, color.b, 0.0f));
		theta = b2_pi / 8.0f * (i + 1);
		v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		DirectX::VertexPositionColor processedVertex2(DirectX::XMFLOAT3(v.x, v.y, 0), DirectX::XMFLOAT4(color.r, color.g, color.b, 0.0f));
		m_renderer.DrawTriangle(processedVertex0, processedVertex1, processedVertex2);
	}
	m_renderer.End();
	
	m_d3dContext->OMSetBlendState(m_commonStates.Opaque(), NULL, 0xffffffff);
	DrawCircle(center, radius, color);

	b2Vec2 p = center + radius * axis;
	DrawSegment(center, p, color);
	//const float32 k_segments = 16.0f;
	//const float32 k_increment = 2.0f * b2_pi / k_segments;
	//float32 theta = 0.0f;
	//glEnable(GL_BLEND);
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glColor4f(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);
	//glBegin(GL_TRIANGLE_FAN);
	//for (int32 i = 0; i < k_segments; ++i)
	//{
	//	b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
	//	glVertex2f(v.x, v.y);
	//	theta += k_increment;
	//}
	//glEnd();
	//glDisable(GL_BLEND);

	//theta = 0.0f;
	//glColor4f(color.r, color.g, color.b, 1.0f);
	//glBegin(GL_LINE_LOOP);
	//for (int32 i = 0; i < k_segments; ++i)
	//{
	//	b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
	//	glVertex2f(v.x, v.y);
	//	theta += k_increment;
	//}
	//glEnd();

	//b2Vec2 p = center + radius * axis;
	//glBegin(GL_LINES);
	//glVertex2f(center.x, center.y);
	//glVertex2f(p.x, p.y);
	//glEnd();
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	m_renderer.Begin();
	m_renderer.DrawLine(
		DirectX::VertexPositionColor(DirectX::XMFLOAT3(p1.x, p1.y, 0), DirectX::XMFLOAT4(color.r, color.g, color.b, 1)),
		DirectX::VertexPositionColor(DirectX::XMFLOAT3(p2.x, p2.y, 0), DirectX::XMFLOAT4(color.r, color.g, color.b, 1))
	);
	m_renderer.End();
	//glColor3f(color.r, color.g, color.b);
	//glBegin(GL_LINES);
	//glVertex2f(p1.x, p1.y);
	//glVertex2f(p2.x, p2.y);
	//glEnd();
}

void DebugDraw::DrawTransform(const b2Transform& xf)
{
	b2Vec2 p1 = xf.p, p2;
	const float32 k_axisScale = 10.0f;
	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	m_renderer.Begin();
	m_renderer.DrawLine(
		DirectX::VertexPositionColor(DirectX::XMFLOAT3(p1.x, p1.y, 0), DirectX::XMFLOAT4(1, 0, 0, 1)),
		DirectX::VertexPositionColor(DirectX::XMFLOAT3(p2.x, p2.y, 0), DirectX::XMFLOAT4(1, 0, 0, 1))
	);
	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	m_renderer.DrawLine(
		DirectX::VertexPositionColor(DirectX::XMFLOAT3(p1.x, p1.y, 0), DirectX::XMFLOAT4(0, 1, 0, 1)),
		DirectX::VertexPositionColor(DirectX::XMFLOAT3(p2.x, p2.y, 0), DirectX::XMFLOAT4(0, 1, 0, 1))
	);
	m_renderer.End();
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
	DirectX::XMFLOAT4 processedColor(color.r, color.g, color.b, 1);
	DirectX::XMFLOAT3 lowerLeft(p.x - halfSize, p.y - halfSize, 0);
	DirectX::XMFLOAT3 upperLeft(p.x - halfSize, p.y + halfSize, 0);
	DirectX::XMFLOAT3 lowerRight(p.x + halfSize, p.y - halfSize, 0);
	DirectX::XMFLOAT3 upperRight(p.x + halfSize, p.y + halfSize, 0);
	m_renderer.Begin();
	m_renderer.DrawQuad(
		DirectX::VertexPositionColor(lowerLeft, processedColor),
		DirectX::VertexPositionColor(lowerRight, processedColor),
		DirectX::VertexPositionColor(upperRight, processedColor),
		DirectX::VertexPositionColor(upperLeft, processedColor)
	);
	m_renderer.End();
	//glPointSize(size);
	//glBegin(GL_POINTS);
	//glColor3f(color.r, color.g, color.b);
	//glVertex2f(p.x, p.y);
	//glEnd();
	//glPointSize(1.0f);
}

void DebugDraw::DrawString(int x, int y, const char *string, ...)
{
	//char buffer[128];

	//va_list arg;
	//va_start(arg, string);
	//vsprintf(buffer, string, arg);
	//va_end(arg);

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
	//char buffer[128];

	//va_list arg;
	//va_start(arg, string);
	//vsprintf(buffer, string, arg);
	//va_end(arg);

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
	DirectX::XMFLOAT4 processedColor(c.r, c.g, c.b, 1);
	DirectX::XMFLOAT3 lowerLeft(center.x - extents.x, center.y - extents.y, 0);
	DirectX::XMFLOAT3 upperLeft(center.x - extents.x, center.y + extents.y, 0);
	DirectX::XMFLOAT3 lowerRight(center.x + extents.x, center.y - extents.y, 0);
	DirectX::XMFLOAT3 upperRight(center.x + extents.x, center.y + extents.y, 0);
	DirectX::VertexPositionColor vertices[5] = {
		DirectX::VertexPositionColor(lowerLeft, processedColor),
		DirectX::VertexPositionColor(lowerRight, processedColor),
		DirectX::VertexPositionColor(upperRight, processedColor),
		DirectX::VertexPositionColor(upperLeft, processedColor),
		DirectX::VertexPositionColor(lowerLeft, processedColor)
	};
	m_renderer.Begin();
	m_renderer.Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, vertices, 5);
	m_renderer.End();
	//glColor3f(c.r, c.g, c.b);
	//glBegin(GL_LINE_LOOP);
	//glVertex2f(aabb->lowerBound.x, aabb->lowerBound.y);
	//glVertex2f(aabb->upperBound.x, aabb->lowerBound.y);
	//glVertex2f(aabb->upperBound.x, aabb->upperBound.y);
	//glVertex2f(aabb->lowerBound.x, aabb->upperBound.y);
	//glEnd();
}
