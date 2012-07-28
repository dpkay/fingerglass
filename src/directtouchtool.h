#ifndef directtouchtool_h__
#define directtouchtool_h__

#include "tool.h"

class DirectTouchTool : public Tool
{
public:
	DirectTouchTool(SettingContainer * setting_container) :
		Tool(setting_container) {}

	void updateAfterTouchEvent(const QTouchEvent * event)
	{
		Q_UNUSED(event);
	}

	bool allowRigidWidgetInteraction(const SceneTouchPoint & stp, const RigidWidgetController & rwc) const
	{
		Q_UNUSED(stp);
		Q_UNUSED(rwc);
		return true;
	}
};


#endif // directtouchtool_h__