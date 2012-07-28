#include "precisionhandlewidgetrenderer.h"

// --------------------------------------------------------
void PrecisionHandleWidgetRenderer::paint( GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer ) const
{
	Q_UNUSED(to_texture_renderer);
	if(pass == 0)
	{
		//Q_ASSERT(controller()->magnifyingGlasses().size() == 2);
		//drawMagnifyingGlass(container, controller()->magnifyingGlasses().back());
		//QSizeF tip_size(controller()->tipRadius()*2, controller()->tipRadius()*2);
		//drawTexturedQuad(container->texture("precision_cross"), controller()->tipPoint(), tip_size);

		drawHollowCircle(container, controller()->tipPoint(), controller()->tipRadius());

		//float aspect_ratio = 2*controller()->magnifyingGlassRadius()/QLineF(controller()->gripPoint(), controller()->tipPoint()).length();
		//drawPrecisionHandle(container, controller()->gripPoint(), controller()->tipPoint(), aspect_ratio);
		drawMagnifyingGlass(container, controller()->gripMagnifyingGlass());

		/*drawTexturedQuad(container->texture("precision_cross"), controller()->tipPoint(), cross_size);*/
		//drawTexturedQuad(container->texture("precision_cross"), controller()->gripPoint(), cross_size);
		drawCross(container, controller()->tipPoint());
		drawCross(container, controller()->gripPoint());
	}
}

// --------------------------------------------------------
const PrecisionHandleWidgetController * PrecisionHandleWidgetRenderer::controller() const
{
	const PrecisionHandleWidgetController * phwc = dynamic_cast<const PrecisionHandleWidgetController *>(TouchWidgetRenderer::controller());
	Q_ASSERT(phwc != NULL);
	return phwc;
}



//return upcastedController<PrecisionHandleWidgetController>(rawController());