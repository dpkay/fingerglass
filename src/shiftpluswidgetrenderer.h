#ifndef shiftpluswidgetrenderer_h__
#define shiftpluswidgetrenderer_h__

#include "touchwidgetrenderer.h"
#include "shiftpluswidgetcontroller.h"

class ShiftPlusWidgetRenderer : public TouchWidgetRenderer
{
public:
	ShiftPlusWidgetRenderer(ShiftPlusWidgetController * controller);
	void paint(GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer) const;

	virtual unsigned int numPasses() const { return 1; }
	const ShiftPlusWidgetController * controller() const; 
	virtual bool isSceneDrawer() const { return true; }
};

#endif // shiftwidgetrenderer_h__