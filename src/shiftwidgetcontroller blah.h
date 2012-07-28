#ifndef shiftwidgetcontroller_h__
#define shiftwidgetcontroller_h__

#include "touchwidgetcontroller.h"
#include "magnifyingglassmanager.h"
#include "settingcontainer.h"

class ShiftWidgetController : public TouchWidgetController
{
public:
	ShiftWidgetController(const SettingContainer & tool_setting_container);
	~ShiftWidgetController();

	// from TouchWidgetController
	virtual void touchPointPressed(SceneTouchPoint * p) { Q_UNUSED(p); }
	virtual void touchPointMoved(SceneTouchPoint * p) { Q_UNUSED(p); _request_update = true; }
	virtual void touchPointReleased(SceneTouchPoint * p);
	virtual bool acceptTouchPoint(const SceneTouchPoint & p) const { Q_UNUSED(p); return false; }
	virtual bool forwardTouchPoint(const SceneTouchPoint & p) const { Q_UNUSED(p); return true; }

	// from Timed
	virtual void timeStep();

	// magnifying glass management
	const QVector<MagnifyingGlass *> & magnifyingGlasses() const { return _mgm->magnifyingGlasses(); }
	void updateMagnifyingGlasses();
	void setDefiningTouch(SceneTouchPoint * stp) { _defining_point = stp; }

private:
	const SettingContainer & _tool_setting_container;
	MagnifyingGlassManager * _mgm;
	SceneTouchPoint * _defining_point;
	bool _request_update;

};


#endif // shiftwidgetcontroller_h__