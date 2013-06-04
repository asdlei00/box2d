#include "pch.h"
#include "RenderWinRT.h"
#include "Effects.h"
#include <d3d11.h>
#include "TestRenderer.h"



DebugDraw::DebugDraw()
{

}

void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	const int32 totalVertexCount = vertexCount + 1;
	const b2Vec2 *currentVertex = vertices;
	D2D1_POINT_2F *processedVertices = new D2D1_POINT_2F[totalVertexCount];
	D2D1_POINT_2F *currentProcessedVertex = processedVertices;
	D2D1_COLOR_F  d2dColor=D2D1::ColorF(color.r,color.g,color.b,1.0f);
	for(int32 i = 0; i < vertexCount; ++i)
	{
		currentProcessedVertex->x = currentVertex->x;
		currentProcessedVertex->y = currentVertex->y;
		++currentVertex;
		++currentProcessedVertex;
	}
	currentProcessedVertex->x = vertices->x;
	currentProcessedVertex->y = vertices->y;
	testRenderer->DrawLineList(processedVertices,totalVertexCount,d2dColor);
	delete [] processedVertices;
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	D2D1_POINT_2F firstProcessedVertex=D2D1::Point2F(vertices->x,vertices->y);
	D2D1_COLOR_F d2dColor=D2D1::ColorF(color.r*0.5f,color.g*0.5f,color.b*0.5f,1.0f);
	const b2Vec2 *currentVertex = vertices + 1;
	for(int32 i = 2; i < vertexCount; ++i)
	{
		D2D1_POINT_2F processedVertex1=D2D1::Point2F(currentVertex->x,currentVertex->y);
		++currentVertex;
		D2D1_POINT_2F processedVertex2=D2D1::Point2F(currentVertex->x,currentVertex->y);		
		testRenderer->DrawTriangle(firstProcessedVertex,processedVertex1,processedVertex2,d2dColor);
	}
	DrawPolygon(vertices, vertexCount, color);
}

void DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	for(int32 i = 0; i < 16; ++i)
	{
		float theta = b2_pi / 8.0f * i;
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		D2D1_POINT_2F processedVertex0=D2D1::Point2F(v.x, v.y);
		D2D1_COLOR_F d2dColor=D2D1::ColorF(color.r, color.g, color.b, 1);
		theta = b2_pi / 8.0f * (i + 1);
		v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		D2D1_POINT_2F processedVertex1=D2D1::Point2F(v.x, v.y);
		testRenderer->DrawLine(processedVertex0, processedVertex1,d2dColor);
	}
}

void DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	D2D1_POINT_2F processedVertex0=D2D1::Point2F(center.x, center.y);
	D2D1_COLOR_F d2dColor=D2D1::ColorF(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, 0.5f);
	for(int32 i = 0; i < 16; ++i)
	{
		float theta = b2_pi / 8.0f * i;
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		D2D1_POINT_2F processedVertex1=D2D1::Point2F(v.x, v.y);
		theta = b2_pi / 8.0f * (i + 1);
		v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		D2D1_POINT_2F processedVertex2=D2D1::Point2F(v.x, v.y);		
		testRenderer->DrawTriangle(processedVertex0, processedVertex1, processedVertex2,d2dColor);
	}
	DrawCircle(center,radius,color);
	b2Vec2 p = center + radius * axis;
	DrawSegment(center, p, color);
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	D2D1_COLOR_F d2dColor=D2D1::ColorF(color.r, color.g, color.b, 1);
	testRenderer->DrawLine(D2D1::Point2F(p1.x, p1.y),D2D1::Point2F(p2.x, p2.y),d2dColor);
}

void DebugDraw::DrawTransform(const b2Transform& xf)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	D2D1_COLOR_F d2dColor=D2D1::ColorF(1, 0, 0, 1);
	b2Vec2 p1 = xf.p, p2;
	const float32 k_axisScale = 10.0f;
	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	testRenderer->DrawLine(D2D1::Point2F(p1.x, p1.y),D2D1::Point2F(p2.x, p2.y), d2dColor);
	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	d2dColor=D2D1::ColorF(1, 0, 0, 1);
	testRenderer->DrawLine(D2D1::Point2F(p1.x, p1.y),D2D1::Point2F(p2.x, p2.y), d2dColor);
}

void DebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	float halfSize = size * 0.5f;
	D2D1_COLOR_F processedColor=D2D1::ColorF(color.r, color.g, color.b, 1);
	D2D1_POINT_2F lowerLeft=D2D1::Point2F(p.x - halfSize, p.y - halfSize);
	D2D1_POINT_2F upperLeft=D2D1::Point2F(p.x - halfSize, p.y + halfSize);
	D2D1_POINT_2F lowerRight=D2D1::Point2F(p.x + halfSize, p.y - halfSize);
	D2D1_POINT_2F upperRight=D2D1::Point2F(p.x + halfSize, p.y + halfSize);
	testRenderer->DrawQuad(lowerLeft,lowerRight,upperRight,upperLeft,processedColor);
}

void DebugDraw::DrawString(int x, int y, const char *string, ...)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	if(testRenderer->GetTextEnable())
	{			
		char buffer[128];
		va_list arg;
		va_start(arg, string);
		vsprintf_s(buffer, string, arg);
		va_end(arg);
		wchar_t wbuffer[128];
		size_t printed;
		mbstowcs_s(&printed, wbuffer, buffer, sizeof(wbuffer));
		testRenderer->PrintString(wbuffer,x,y,D2D1::ColorF(0.9f, 0.6f, 0.6f, 1.0f));
	}
}

void DebugDraw::DrawString(const b2Vec2& p, const char *string, ...)
{
	DrawString((int)p.x,(int)p.y,string);
}


void DebugDraw::DrawAABB(b2AABB* aabb, const b2Color& c)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	b2Vec2 center(aabb->GetCenter());
	b2Vec2 extents(aabb->GetExtents());
	D2D1_COLOR_F processedColor=D2D1::ColorF(c.r, c.g, c.b, 1);
	D2D1_POINT_2F lowerLeft=D2D1::Point2F(center.x - extents.x, center.y - extents.y);
	D2D1_POINT_2F upperLeft=D2D1::Point2F(center.x - extents.x, center.y + extents.y);
	D2D1_POINT_2F lowerRight=D2D1::Point2F(center.x + extents.x, center.y - extents.y);
	D2D1_POINT_2F upperRight=D2D1::Point2F(center.x + extents.x, center.y + extents.y);
	D2D1_POINT_2F vertices[5] = {lowerLeft,lowerRight,upperRight,upperLeft,lowerLeft};
	testRenderer->DrawLineList(vertices,5,processedColor);
}
