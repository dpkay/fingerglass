#ifndef magnifyingwidgetrenderer_h__
#define magnifyingwidgetrenderer_h__

#include "touchwidgetrenderer.h"
#include "magnifyingwidgetcontroller.h"
#include "warping.h"

class MagnifyingWidgetRenderer : public TouchWidgetRenderer
{
public:
	MagnifyingWidgetRenderer(MagnifyingWidgetController * controller);
	void paint(GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer) const;
	virtual unsigned int numPasses() const { return 1; }
	const MagnifyingWidgetController * controller() const; 
	bool isSceneDrawer() const { return true; }

private:
	QList<float> _resizer_angles;

};

#endif // magnifyingwidgetrenderer_h__