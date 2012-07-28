#ifndef touchwidgetrenderer_h__
#define touchwidgetrenderer_h__

#include <GL/glew.h>
//#include <QtOpenGL>
#include "warping.h"
#include "touchwidgetcontroller.h"
#include "glresourcecontainer.h"
#include "magnifyingglass.h"

class TouchWidgetRenderer : public QObject
{
Q_OBJECT

public:
	TouchWidgetRenderer(TouchWidgetController * controller, float alpha = 0.0f);
	~TouchWidgetRenderer();

	virtual void paint(GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer) const = 0;
	virtual const TouchWidgetController * controller() const { return _controller; } 
	virtual bool isSceneDrawer() const { return false; }
	virtual unsigned int numPasses() const = 0;

signals:
	void drawScene() const;
	void setupViewport() const;

public:
//protected:
	void drawUnitQuad(float alpha) const;
	void drawQuad(const QPointF & pos, const QSizeF & size, float rotation, float alpha) const;
	void drawTexturedUnitQuad(GLuint tex_id, float alpha) const;
	void drawTexturedQuad(GLuint tex_id, const QPointF & pos, const QSizeF & size, float rotation = 0.0f, float alpha = 1.0f) const;
	void drawSceneToFbo(QGLFramebufferObject * fbo, const QPointF & pos, const QSizeF & size, float rotation) const;

	void drawMagnifyingGlass(GLResourceContainer * container,
		const MagnifyingGlass * mg,
		float alpha = 1.0f) const;

	void drawPrecisionHandle(GLResourceContainer * container, const QPointF & grip, const QPointF & tip, float aspect_ratio = 0.2f, float alpha = 1.0f) const;

	void drawCross(GLResourceContainer * container, const QPointF & pos, const QSizeF size = QSizeF(0.015f, 0.015f)) const;

	void drawHollowCircle( GLResourceContainer * container, const QPointF & pos, qreal radius, qreal alpha = 1.0f, const QColor & color = Qt::white, qreal thickness = 0.001f, qreal blur_multiplier = 25 ) const;

	void drawResizers(
		GLResourceContainer * container,
		const SceneTouchPoint * resizing_point,
		const QPointF & circle_center,
		float circle_radius,
		const QPointF & base_pos,
		QList<float> resizer_angles ) const;

protected:
	float alpha() const;
	//virtual float initialAlpha() const;

private:
	TouchWidgetController * _controller;
	Warping<qreal> _alpha;

	//static GLuint _unit_square_vbo;

	static GLuint _current_texture_id;

};

#endif // touchwidgetrenderer_h__