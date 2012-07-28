#ifndef precisionhandletool_h__
#define precisionhandletool_h__

#include "tool.h"

class PrecisionHandleTool : public Tool
{
public:
	PrecisionHandleTool(SettingContainer * setting_container) : Tool(setting_container) {}
	virtual void updateAfterTouchEvent(const QTouchEvent * event);
	virtual bool allowRigidWidgetInteraction(const SceneTouchPoint & stp, const RigidWidgetController & rwc) const;

};

#endif // shifttool_h__