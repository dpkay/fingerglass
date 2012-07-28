#ifndef fingerglasstool_h__
#define fingerglasstool_h__

#include "tool.h"
#include "settingcontainer.h"
#include "fingerglassopener.h"

class FingerGlassTool : public Tool
{
	Q_OBJECT
public:
	FingerGlassTool(SettingContainer * setting_container);
	virtual void updateAfterTouchEvent(const QTouchEvent * event);
	virtual bool allowRigidWidgetInteraction(const SceneTouchPoint & stp, const RigidWidgetController & rwc) const;
	
protected slots:
	//void createMagnifyingWidgetIfNecessary();
	void openerTimedOut(const QList<SceneTouchPoint *> & touches);
	void createHandleWidgetIfNecessary();
	void createMagnifyingWidgetFromHandleWidget();

protected:
	bool allowMagnifyingWidget();

private:
	FingerGlassOpener * _opener;

};

#endif // fingerglasstool_h__