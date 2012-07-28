#include "dualfingerstretchwidgetrenderer.h"

// --------------------------------------------------------
void DualFingerStretchWidgetRenderer::paint( GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer ) const
{
	Q_UNUSED(to_texture_renderer);
	if(pass == 0)
	{
		const MagnifyingGlass * mg = controller()->magnifyingGlass();
		drawResizers(container, controller()->resizingPoint(), mg->dstCenter(), mg->dstRadius(), mg->dstCenter()-QPointF(1,0), _resizer_angles);
		
		float alpha = 1.0f;
		if(fabsf(mg->ratio()-1.0f) < 0.01f)
			alpha = 0.5f;
		drawMagnifyingGlass(container, mg, alpha);
	}
}

// --------------------------------------------------------
const DualFingerStretchWidgetController * DualFingerStretchWidgetRenderer::controller() const
{
	const DualFingerStretchWidgetController * dfswc = dynamic_cast<const DualFingerStretchWidgetController *>(TouchWidgetRenderer::controller());
	Q_ASSERT(dfswc != NULL);
	return dfswc;
}

// --------------------------------------------------------
DualFingerStretchWidgetRenderer::DualFingerStretchWidgetRenderer( DualFingerStretchWidgetController * controller ) :
TouchWidgetRenderer(controller)
{
	_resizer_angles << 120 << 240 << 360;
}