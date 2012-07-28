#ifndef handlewidgetrenderer_h__
#define handlewidgetrenderer_h__

#include "touchwidgetrenderer.h"
#include "handlewidgetcontroller.h"


class HandleWidgetRenderer : public TouchWidgetRenderer
{
public:
	HandleWidgetRenderer(HandleWidgetController * controller);
	void paint(GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer) const;

	virtual unsigned int numPasses() const { return 1; }
	const HandleWidgetController * controller() const; 
	bool isSceneDrawer() const { return true; }

private:
	QList<float> _resizer_angles;

};	

#endif // handlewidgetrenderer_h__