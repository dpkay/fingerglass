#include "handlewidgetrenderer.h"

// --------------------------------------------------------
HandleWidgetRenderer::HandleWidgetRenderer( HandleWidgetController * controller ) :
	TouchWidgetRenderer(controller, 1.0f)
{
	_resizer_angles << 0 << 120 << 240;
}

// --------------------------------------------------------
const HandleWidgetController * HandleWidgetRenderer::controller() const
{
	const HandleWidgetController * hwc = dynamic_cast<const HandleWidgetController *>(TouchWidgetRenderer::controller());
	Q_ASSERT(hwc != NULL);
	return hwc;
}

// --------------------------------------------------------
void HandleWidgetRenderer::paint( GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer ) const
{
	Q_UNUSED(to_texture_renderer);
	if(pass == 0)
	{
		static Warping<qreal> hollow_circle_brightness(1.0f, 0.2f);
		hollow_circle_brightness.setTarget(controller()->isLocked() ? 0.5f : 1.0f);

		const AbsoluteHandle * absolute_handle = controller()->absoluteHandle();
		const ClutchHandle * clutch_handle = controller()->clutchHandle();

		Q_ASSERT(absolute_handle != NULL);
		Q_ASSERT(clutch_handle != NULL);
		const MagnifyingGlass * clutch_mg = clutch_handle->magnifyingGlass();

		QPointF line_start = QLineF(absolute_handle->pos(), clutch_mg->dstCenter()).unitVector().pointAt(absolute_handle->radius());
		QPointF line_end = QLineF(clutch_mg->dstCenter(), absolute_handle->pos()).unitVector().pointAt(clutch_mg->dstRadius());

		drawHollowCircle(container, absolute_handle->pos(), absolute_handle->radius(), hollow_circle_brightness.value(), Qt::white);

		//drawHollowCircle(container, );
		if(controller()->showHandle())
		{
			if(QLineF(absolute_handle->pos(), clutch_mg->dstCenter()).length()>clutch_mg->dstRadius()+absolute_handle->radius())
			{
				glBindTexture(GL_TEXTURE_2D, 0);
				glLineWidth(2);
				glColor4f(1,1,1,1);
				glBegin(GL_LINE_STRIP);
				glVertex2f(line_start.x(), line_start.y());
				glVertex2f(line_end.x(), line_end.y());
				glEnd();
			}
			//drawResizers(container, controller()->clutchResizingTouchPoint(), clutch_mg->dstCenter(), clutch_mg->dstRadius(), clutch_mg->dstCenter()-QPointF(1,0), _resizer_angles);
			drawMagnifyingGlass(container, clutch_mg);
		}
	}
}

