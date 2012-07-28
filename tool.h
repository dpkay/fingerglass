#ifndef tool_h__
#define tool_h__

#include "scenetouchpoint.h"
#include "rigidwidgetcontroller.h"
#include "settingcontainer.h"

class TouchWidgetManager;
class TouchPointManager;


class Tool : public QObject
{
public:
	Tool(SettingContainer * setting_container);
	~Tool();
	virtual void setup() {};
	virtual void updateAfterTouchEvent(const QTouchEvent * event) = 0;
	virtual bool allowRigidWidgetInteraction(const SceneTouchPoint & stp, const RigidWidgetController & rwc) const;

	// managers
	void setTouchWidgetManager(TouchWidgetManager * twm) { _twm = twm; }
	void setTouchPointManager(TouchPointManager * tpm) { _tpm = tpm; }

protected:
	const SettingContainer & settingContainer() const { return *this->_setting_container; }
	const TouchWidgetManager * touchWidgetManager() const { return _twm; }
	TouchWidgetManager * touchWidgetManager() { return _twm; }
	TouchPointManager * touchPointManager() { return _tpm; }
	SceneTouchPoint * getFreeTouch();
	bool getFreeTouchPair(QPair<SceneTouchPoint *, SceneTouchPoint *> * out_pair);

private:
	SettingContainer * _setting_container;
	TouchWidgetManager * _twm;
	TouchPointManager * _tpm;

	friend QDebug operator<<(QDebug dbg, const Tool &t);

};

#endif // tool_h__o