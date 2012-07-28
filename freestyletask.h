#ifndef freestyletask_h__
#define freestyletask_h__

#include "task.h"

class FreestyleTask : public Task
{
public:
	FreestyleTask(SettingContainer * setting_container) : Task(setting_container) {}

	virtual QString caption() const { return "Freestyle mode."; }
	virtual QString prefix() const { return "free";}
	virtual void startRound() {}
	virtual void stopRound() {}
	virtual void highlightCurrentBall() {}

	virtual void touchPointPressed(SceneTouchPoint * stp) { Q_UNUSED(stp); }
	virtual void touchPointMoved(SceneTouchPoint * stp) { Q_UNUSED(stp); }
	virtual void touchPointReleased(SceneTouchPoint * stp) { Q_UNUSED(stp); }
};

#endif // freestyletask_h__