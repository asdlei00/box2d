#pragma once

#include <Box2D/Box2D.h>
#include <d3d11.h>

struct b2AABB;
namespace DirectX {
struct VertexPositionColor;
template <typename T> class PrimitiveBatch;
class CommonStates;
class BasicEffect;
}

class DebugDraw : public b2Draw
{
public:
	DebugDraw();

	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);

	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);

	void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);

	void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);

	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);

	void DrawTransform(const b2Transform& xf);

    void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color);

    void DrawString(int x, int y, const char* string, ...); 

    void DrawString(const b2Vec2& p, const char* string, ...);

    void DrawAABB(b2AABB* aabb, const b2Color& color);

private:
	DirectX::CommonStates *m_commonStates;
	DirectX::BasicEffect *m_basicEffect;
	DirectX::PrimitiveBatch<DirectX::VertexPositionColor> *m_renderer;
	ID3D11DeviceContext *m_d3dContext;
};