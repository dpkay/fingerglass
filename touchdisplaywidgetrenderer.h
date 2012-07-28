#ifndef touchdisplaywidgetrenderer_h__
#define touchdisplaywidgetrenderer_h__

#include "touchwidgetrenderer.h"
#include "touchpointmanager.h"
#include "touchwidgetmanager.h"
#include "rigidwidgetcontroller.h"
#include "settingcontainer.h"

class TouchDisplayWidgetRenderer : public TouchWidgetRenderer 
{
public:
	TouchDisplayWidgetRenderer(RigidWidgetController * controller, TouchPointManager * tpm, TouchWidgetManager * twm, const SettingContainer & setting_container);
	void paint(GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer) const;
	void setScreenSize(const QSize & screen_size);
	bool isSceneDrawer() const { return false; }
	virtual unsigned int numPasses() const { return 1; }

private:
	TouchPointManager * _tpm;
	TouchWidgetManager * _twm;
	const SettingContainer & _tool_setting_container;
	//QSize _screen_size;

};

#endif // touchdisplaywidgetcontroller_h__