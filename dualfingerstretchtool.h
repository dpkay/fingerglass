#ifndef dualfingerstretchtool_h__
#define dualfingerstretchtool_h__

#include "tool.h"

class DualFingerStretchTool : public Tool
{
public:
	DualFingerStretchTool(SettingContainer * setting_container) : Tool(setting_container) {}
	virtual void updateAfterTouchEvent(const QTouchEvent * event);
	virtual bool allowRigidWidgetInteraction(const SceneTouchPoint & stp, const RigidWidgetController & rwc) const;

};

#endif // shifttool_h__