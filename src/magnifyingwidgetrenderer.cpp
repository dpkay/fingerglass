#include "magnifyingwidgetrenderer.h"
#include "warping.h"

// --------------------------------------------------------
MagnifyingWidgetRenderer::MagnifyingWidgetRenderer( MagnifyingWidgetController * controller ) :
	TouchWidgetRenderer(controller)
//	_resizer_activeness(0.0f,0.2f)
{
	_resizer_angles << 120 << 240;
}

// --------------------------------------------------------
void MagnifyingWidgetRenderer::paint(
	GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer) const
{
	Q_UNUSED(to_texture_renderer);
	if(pass == 0)
	{
		static Warping<qreal> hollow_circle_brightness(1.0f, 0.2f);
		hollow_circle_brightness.setTarget(controller()->locked() ? 0.5f : 1.0f);

		const MagnifyingGlass * base_glass = controller()->magnifyingGlasses().first();
		const MagnifyingGlass * zoom_glass = controller()->magnifyingGlasses().last();
		const SceneTouchPoint * resizing_point = controller()->resizingTouchPoint();
		
		//float brightness = controller()->innerTouchIds().empty() ? 1.0f : 0.8f;
		//QColor color(hollow_circle_brightness.value()*255,hollow_circle_brightness.value()*255,hollow_circle_brightness.value()*255);
		QColor color = Qt::white;
		if(resizing_point != NULL)
		{
			color.setNamedColor("#f90");
		}
		//else
		//{
		//	color.setHslF(0,0,hollow_circle_brightness.value(),1);
		//}
		drawHollowCircle(container, base_glass->dstCenter(), base_glass->dstRadius(), hollow_circle_brightness.value(), color);

		if(controller()->showGlass())
		{
			if(QLineF(base_glass->dstCenter(), zoom_glass->dstCenter()).length()>zoom_glass->dstRadius()+base_glass->dstRadius())
			{
				QPointF line_start = QLineF(base_glass->dstCenter(), zoom_glass->dstCenter()).unitVector().pointAt(base_glass->dstRadius());
				QPointF line_end = QLineF(zoom_glass->dstCenter(), base_glass->dstCenter()).unitVector().pointAt(zoom_glass->dstRadius());
				glBindTexture(GL_TEXTURE_2D, 0);
				glLineWidth(2);
				glColor4f(1,1,1,1);
				glBegin(GL_LINE_STRIP);
				glVertex2f(line_start.x(), line_start.y());
				glVertex2f(line_end.x(), line_end.y());
				glEnd();
			}
			//drawResizers(container, resizing_point, zoom_glass->dstCenter(), zoom_glass->dstRadius(), base_glass->dstCenter(), _resizer_angles);
			drawMagnifyingGlass(container, zoom_glass);
		}
		else
		{
			QPointF base_pos = QLineF(base_glass->dstCenter(), zoom_glass->dstCenter()).unitVector().pointAt(base_glass->dstRadius()*1.07f);
			float rotation = QLineF(base_glass->dstCenter(), zoom_glass->dstCenter()).angleTo(QLineF(0,0,1,0)) + 90;
			QSizeF size = QSizeF(1,1) * base_glass->dstRadius()*0.6f;
			drawTexturedQuad(container->texture("mglass_hidden_arrow_head"), base_pos, size, rotation, hollow_circle_brightness.value()*0.35f);
			drawTexturedQuad(container->texture("mglass_hidden_arrow_head"), base_pos, size/zoom_glass->ratio(), rotation, hollow_circle_brightness.value());

			drawCross(container, base_glass->dstCenter());
			drawCross(container, zoom_glass->dstCenter());
		}
	}
}

// --------------------------------------------------------
const MagnifyingWidgetController * MagnifyingWidgetRenderer::controller() const
{
	const MagnifyingWidgetController * mwc = dynamic_cast<const MagnifyingWidgetController *>(TouchWidgetRenderer::controller());
	Q_ASSERT(mwc != NULL);
	return mwc;
}
