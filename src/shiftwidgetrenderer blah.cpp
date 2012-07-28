#include "shiftwidgetrenderer.h"

// --------------------------------------------------------
ShiftWidgetRenderer::ShiftWidgetRenderer( ShiftWidgetController * controller ) :
	TouchWidgetRenderer(controller)
{
}

// --------------------------------------------------------
void ShiftWidgetRenderer::paint( GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer ) const
{
	Q_UNUSED(to_texture_renderer);
	if(pass == 0)
	{
		Q_ASSERT(controller()->magnifyingGlasses().size() == 2);
		const MagnifyingGlass * mg = controller()->magnifyingGlasses().back();
		drawMagnifyingGlass(container, mg);
		drawCross(container, mg->dstCenter());
		drawCross(container, mg->srcCenter());
		//drawWhiteCircle(container, mg->srcCenter(), mg->srcRadius());
		//drawMagnifyingGlass(container, controller()->magnifyingGlasses().front());
	}
}

// --------------------------------------------------------
const ShiftWidgetController * ShiftWidgetRenderer::controller() const
{
	const ShiftWidgetController * swc = dynamic_cast<const ShiftWidgetController *>(TouchWidgetRenderer::controller());
	Q_ASSERT(swc != NULL);
	return swc;
}

