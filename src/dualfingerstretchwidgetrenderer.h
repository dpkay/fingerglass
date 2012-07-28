#ifndef dualfingerstretchwidgetrenderer_h__
#define dualfingerstretchwidgetrenderer_h__

#include "touchwidgetrenderer.h"
#include "dualfingerstretchwidgetcontroller.h"

class DualFingerStretchWidgetRenderer : public TouchWidgetRenderer
{
public:
	// constructor
	DualFingerStretchWidgetRenderer(DualFingerStretchWidgetController * controller);

	// required for painting
	virtual void paint(GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer) const;
	virtual unsigned int numPasses() const { return 1; }
	virtual bool isSceneDrawer() const { return true; }

	const DualFingerStretchWidgetController * controller() const; 

private:
	QList<float> _resizer_angles;

};

#endif // dualfingerstretchwidgetrenderer_h__