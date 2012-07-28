#ifndef touchdisplaytool_h__
#define touchdisplaytool_h__

#include "tool.h"
#include "afterglowbubble.h"
#include "touchdisplaywidgetrenderer.h"

class TouchDisplayTool : public Tool
{
public:
	TouchDisplayTool(SettingContainer * setting_container) : Tool(setting_container)
	{
		AfterGlowBubble::setLifeSpan(setting_container->intProperty("release_delay"));
	}
	virtual void updateAfterTouchEvent(const QTouchEvent * event) { Q_UNUSED(event); }
	virtual void setup()
	{
		RigidWidgetController * rwc = new RigidWidgetController(TouchDisplayWidgetType);
		TouchDisplayWidgetRenderer * tdwr = new TouchDisplayWidgetRenderer(rwc, touchPointManager(), touchWidgetManager(), settingContainer());
		TouchWidget * tw = new TouchWidget(rwc, tdwr);
		//qDebug() << "created" << tw;
		touchWidgetManager()->add(tw);
	}

};

#endif // touchdisplaytool_h__