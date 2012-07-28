#ifndef precisionhandlewidgetrenderer_h__
#define precisionhandlewidgetrenderer_h__

#include "touchwidgetrenderer.h"
#include "precisionhandlewidgetcontroller.h"

class PrecisionHandleWidgetRenderer : public TouchWidgetRenderer
{
public:
	PrecisionHandleWidgetRenderer(PrecisionHandleWidgetController * controller) :
	  TouchWidgetRenderer(controller) {}
	void paint(GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer) const;

	virtual unsigned int numPasses() const { return 1; }
	const PrecisionHandleWidgetController * controller() const; 
	virtual bool isSceneDrawer() const { return true; }
};

#endif // shiftwidgetrenderer_h__