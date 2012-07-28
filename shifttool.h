#ifndef shifttool_h__
#define shifttool_h__

#include "tool.h"

class ShiftTool : public Tool
{
public:
	ShiftTool(SettingContainer * setting_container) : Tool(setting_container) {}
	virtual void updateAfterTouchEvent(const QTouchEvent * event);
	virtual bool allowRigidWidgetInteraction(const SceneTouchPoint & stp, const RigidWidgetController & rwc) const;

};

#endif // shifttool_h__