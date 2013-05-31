#include "pch.h"
#include "RenderWinRT.h"
#include "Effects.h"
#include <d3d11.h>
#include "TestRenderer.h"


//Helper matrix function to perform our own projection matrix.
D2D1_POINT_2F TransformPoint(DirectX::XMMATRIX* matrix,D2D1_POINT_2F* point,float width,float height)
{
	D2D1_POINT_2F pr={};
	
	float x = point->x*matrix->r[0].m128_f32[0]+
		      point->y*matrix->r[0].m128_f32[1]+
			  matrix->r[0].m128_f32[2]+
			  matrix->r[0].m128_f32[3];

	float y = point->x*matrix->r[1].m128_f32[0]+
		      point->y*matrix->r[1].m128_f32[1]+
			  matrix->r[1].m128_f32[2]+
			  matrix->r[1].m128_f32[3];

	//Coordinate should be in range of -1 to 1.
	pr.x=(x*0.5f+0.5f)*width;
	pr.y=height-((y*0.5f+0.5f)*height);
	return pr;
}
DebugDraw::DebugDraw()
{

}

void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> deviceContext = testRenderer->Get2DDeviceContext();
	D2D_SIZE_F size = deviceContext->GetSize();
	ID2D1SolidColorBrush* solidBrushColor=NULL;
	D2D_COLOR_F solidColor = D2D1::ColorF(color.r,color.g,color.b,1.0f);
	deviceContext->CreateSolidColorBrush(solidColor,&solidBrushColor);
	DirectX::XMMATRIX* projection = testRenderer->GetProjectionMatrix();
	D2D1_POINT_2F point0={};
	D2D1_POINT_2F point1={};
	for(int32 i = 0; i < vertexCount; i++)
	{
		point0.x=vertices[i].x;
		point0.y=vertices[i].y;
		point1.x=vertices[i+1].x;
		point1.y=vertices[i+1].y;
		point0 = TransformPoint(projection,&point0,size.width,size.height);
		point1 = TransformPoint(projection,&point1,size.width,size.height);
		deviceContext->DrawLine(point0,point1,solidBrushColor);
		break;
	}
	for(int32 i=2;i<vertexCount;++i)
	{
		point0.x=vertices[i-1].x;
		point0.y=vertices[i-1].y;
		point1.x=vertices[i].x;
		point1.y=vertices[i].y;
		point0 = TransformPoint(projection,&point0,size.width,size.height);
		point1 = TransformPoint(projection,&point1,size.width,size.height);
		deviceContext->DrawLine(point0,point1,solidBrushColor);
	}
	point0.x=vertices[vertexCount-1].x;
	point0.y=vertices[vertexCount-1].y;
	point1.x=vertices[0].x;
	point1.y=vertices[0].y;		
	point0 = TransformPoint(projection,&point0,size.width,size.height);
	point1 = TransformPoint(projection,&point1,size.width,size.height);
	deviceContext->DrawLine(point0,point1,solidBrushColor);
	solidBrushColor->Release();
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	DrawPolygon(vertices, vertexCount, color);
}

void DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> deviceContext = testRenderer->Get2DDeviceContext();
	D2D_SIZE_F size = deviceContext->GetSize();
	ID2D1SolidColorBrush* solidBrushColor=NULL;
	D2D_COLOR_F solidColor = D2D1::ColorF(color.r,color.g,color.b,1.0f);
	deviceContext->CreateSolidColorBrush(solidColor,&solidBrushColor);
	DirectX::XMMATRIX* projection = testRenderer->GetProjectionMatrix();
	D2D1_POINT_2F point0={};
	D2D1_POINT_2F point1={};

	for(int32 i = 0; i < 16; ++i)
	{
		float theta = b2_pi / 8.0f * i;
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		D2D1_POINT_2F processedVertex0;
		processedVertex0.x=v.x;
		processedVertex0.y=v.y;
		processedVertex0=TransformPoint(projection,&processedVertex0,size.width,size.height);
		theta = b2_pi / 8.0f * (i + 1);
		v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		D2D1_POINT_2F processedVertex1;
		processedVertex1.x=v.x;
		processedVertex1.y=v.y;
		processedVertex1=TransformPoint(projection,&processedVertex1,size.width,size.height);
		deviceContext->DrawLine(processedVertex0, processedVertex1,solidBrushColor);
	}
	solidBrushColor->Release();
}

void DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> deviceContext = testRenderer->Get2DDeviceContext();
	D2D_SIZE_F size = deviceContext->GetSize();
	ID2D1SolidColorBrush* solidBrushColor=NULL;
	D2D_COLOR_F solidColor = D2D1::ColorF(color.r,color.g,color.b,1.0f);
	deviceContext->CreateSolidColorBrush(solidColor,&solidBrushColor);
	DirectX::XMMATRIX* projection = testRenderer->GetProjectionMatrix();
	D2D1_POINT_2F point0={};
	D2D1_POINT_2F point1={};

	for(int32 i = 0; i < 16; ++i)
	{
		float theta = b2_pi / 8.0f * i;
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		D2D1_POINT_2F processedVertex0;
		processedVertex0.x=v.x;
		processedVertex0.y=v.y;
		processedVertex0=TransformPoint(projection,&processedVertex0,size.width,size.height);
		theta = b2_pi / 8.0f * (i + 1);
		v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		D2D1_POINT_2F processedVertex1;
		processedVertex1.x=v.x;
		processedVertex1.y=v.y;
		processedVertex1=TransformPoint(projection,&processedVertex1,size.width,size.height);
		deviceContext->DrawLine(processedVertex0, processedVertex1,solidBrushColor);
	}
	solidBrushColor->Release();
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> deviceContext = testRenderer->Get2DDeviceContext();
	ID2D1SolidColorBrush* solidBrushColor=NULL;
	D2D1_POINT_2F point0={};
	D2D1_POINT_2F point1={};
	D2D_SIZE_F size = deviceContext->GetSize();
	D2D_COLOR_F solidColor = D2D1::ColorF(color.r,color.g,color.b,1.0f);
	DirectX::XMMATRIX* projection = testRenderer->GetProjectionMatrix();
	deviceContext->CreateSolidColorBrush(solidColor,&solidBrushColor);
	point0.x=p1.x;
	point0.y=p1.y;
	point1.x=p2.x;
	point1.y=p2.y;
	point0=TransformPoint(projection,&point0,size.width,size.height);
	point1=TransformPoint(projection,&point1,size.width,size.height);
	deviceContext->DrawLine(point0,point1,solidBrushColor);
	solidBrushColor->Release();
}

void DebugDraw::DrawTransform(const b2Transform& xf)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> deviceContext = testRenderer->Get2DDeviceContext();
	D2D_SIZE_F size = deviceContext->GetSize();
	ID2D1SolidColorBrush* solidBrushColor=NULL;
	D2D_COLOR_F solidColor = D2D1::ColorF(1,1,1,1.0f);
	deviceContext->CreateSolidColorBrush(solidColor,&solidBrushColor);
	DirectX::XMMATRIX* projection = testRenderer->GetProjectionMatrix();
	D2D1_POINT_2F point0={};
	D2D1_POINT_2F point1={};

	b2Vec2 p1 = xf.p, p2;
	const float32 k_axisScale = 10.0f;
	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	point0.x=p1.x;
	point0.y=p1.y;
	point1.x=p2.x;
	point1.y=p2.y;
	point0 = TransformPoint(projection,&point0,size.width,size.height);
	point1 = TransformPoint(projection,&point1,size.width,size.height);
	deviceContext->DrawLine(point0,point1,solidBrushColor);
	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	point0.x=p1.x;
	point0.y=p1.y;
	point1.x=p2.x;
	point1.y=p2.y;
	point0 = TransformPoint(projection,&point0,size.width,size.height);
	point1 = TransformPoint(projection,&point1,size.width,size.height);
	deviceContext->DrawLine(point0,point1,solidBrushColor);
	solidBrushColor->Release();
}

void DebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
{
	float halfSize = size * 0.5f;
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> deviceContext = testRenderer->Get2DDeviceContext();
	D2D_SIZE_F fsize = deviceContext->GetSize();
	ID2D1SolidColorBrush* solidBrushColor=NULL;
	D2D_COLOR_F solidColor = D2D1::ColorF(color.r, color.g, color.b, 1);
	deviceContext->CreateSolidColorBrush(solidColor,&solidBrushColor);
	DirectX::XMMATRIX* projection = testRenderer->GetProjectionMatrix();

	D2D1_POINT_2F lowerLeft;
	lowerLeft.x=p.x - halfSize;
	lowerLeft.y=p.y - halfSize;
	lowerLeft=TransformPoint(projection,&lowerLeft,fsize.width,fsize.height);

	D2D1_POINT_2F upperLeft;
	upperLeft.x=p.x - halfSize;
	upperLeft.y=p.y + halfSize;
	upperLeft=TransformPoint(projection,&upperLeft,fsize.width,fsize.height);

	D2D1_POINT_2F lowerRight;
	lowerRight.x=p.x + halfSize;
	lowerRight.y=p.y - halfSize;
	lowerRight=TransformPoint(projection,&lowerRight,fsize.width,fsize.height);

	D2D1_POINT_2F upperRight;
	upperRight.x=p.x + halfSize;
	upperRight.y=p.y + halfSize;
	upperRight=TransformPoint(projection,&upperRight,fsize.width,fsize.height);

	deviceContext->DrawRectangle(D2D1::RectF(upperLeft.x,upperLeft.y,lowerRight.x,lowerRight.y),solidBrushColor);
	solidBrushColor->Release();
}

void DebugDraw::DrawString(int x, int y, const char *string, ...)
{
	TestRenderer^ testRenderer = TestRenderer::GetInstance();
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> deviceContext = testRenderer->Get2DDeviceContext();
	Microsoft::WRL::ComPtr<IDWriteFactory1> writeFactory=testRenderer->GetWriteFactory();
	D2D_SIZE_F fsize = deviceContext->GetSize();
	ID2D1SolidColorBrush* solidBrushColor=NULL;
	D2D_COLOR_F solidColor = D2D1::ColorF(1,1,1,1);
	deviceContext->CreateSolidColorBrush(solidColor,&solidBrushColor);
	DirectX::XMMATRIX* projection = testRenderer->GetProjectionMatrix();
	IDWriteTextFormat* textFormatBody=NULL;
	
	//Not a good idea to create it every frame, but this is just to showcased that it works.
	writeFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            24,
            L"en-us",
            &textFormatBody
            );


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
		deviceContext->DrawText(wbuffer,wcslen(wbuffer),textFormatBody,D2D1::RectF((float)x,(float)y,fsize.width,fsize.height),solidBrushColor);
	}
	textFormatBody->Release();
	solidBrushColor->Release();
}

void DebugDraw::DrawString(const b2Vec2& p, const char *string, ...)
{
	DrawString((int)p.x,(int)p.y,string);
}


void DebugDraw::DrawAABB(b2AABB* aabb, const b2Color& c)
{
	b2Vec2 center(aabb->GetCenter());
	b2Vec2 extents(aabb->GetExtents());
	b2Vec2 lowerLeft(center.x - extents.x, center.y - extents.y);
	b2Vec2 upperLeft(center.x - extents.x, center.y + extents.y);
	b2Vec2 lowerRight(center.x + extents.x, center.y - extents.y);
	b2Vec2 upperRight(center.x + extents.x, center.y + extents.y);
	b2Vec2 vertices[5]={lowerLeft,lowerRight,upperRight,upperLeft,upperLeft};
	DrawPolygon(vertices,5,c);
}
