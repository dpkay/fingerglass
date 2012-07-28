#include "shiftpluswidgetrenderer.h"

// --------------------------------------------------------
ShiftPlusWidgetRenderer::ShiftPlusWidgetRenderer( ShiftPlusWidgetController * controller ) :
	TouchWidgetRenderer(controller)
{
}

// --------------------------------------------------------
void ShiftPlusWidgetRenderer::paint( GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer ) const
{
	Q_UNUSED(to_texture_renderer);
	if(pass == 0)
	{
		const MagnifyingGlass * mg = controller()->magnifyingGlass();
		drawCross(container, mg->srcCenter());
		//drawCross(container, controller()->gripPoint());
		if(to_texture_renderer == NULL)
		{
			//drawPrecisionHandle(container, controller()->gripPoint(), mg->srcCenter(), 0.1f, 0.1f);
			drawPrecisionHandle(container, mg->dstCenter(), mg->srcCenter(), 2*mg->dstRadius()/QLineF(mg->dstCenter(),mg->srcCenter()).length(), 0.1f);
		}
		//drawHollowCircle(container,controller()->_mgm->baseGlass()->srcCenterT(), controller()->_mgm->baseGlass()->srcRadiusT());
		//drawMagnifyingGlass(container, controller()->_mgm->baseGlass());

			//void drawHollowCircle( GLResourceContainer * container, const QPointF & pos, qreal radius, qreal alpha = 1.0f, const QColor & color = Qt::white, qreal thickness = 0.001f, qreal blur_multiplier = 25 ) const;
		drawMagnifyingGlass(container, mg);
		drawCross(container, mg->dstCenter());
		if(!controller()->hasDefiningTouch())
		{
			//drawTexturedQuad(container->texture("precision_handle_grip"), controller()->gripPoint(), controller()->gripSize());
		}

		//qDebug() << "drawing shiftplus" << mg->dstCenter() << mg->dstRadius() << mg->dstCenterT() << mg->dstRadiusT();
		//drawWhiteCircle(container, mg->srcCenter(), mg->srcRadius());
		//drawMagnifyingGlass(container, controller()->magnifyingGlasses().front());
	}
}

// --------------------------------------------------------
const ShiftPlusWidgetController * ShiftPlusWidgetRenderer::controller() const
{
	const ShiftPlusWidgetController * swc = dynamic_cast<const ShiftPlusWidgetController *>(TouchWidgetRenderer::controller());
	Q_ASSERT(swc != NULL);
	return swc;
}

