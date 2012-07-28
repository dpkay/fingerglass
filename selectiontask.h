#ifndef selectiontask_h__
#define selectiontask_h__

#include "task.h"
#include "variousattachments.h"

class SelectionTask : public Task
{
public:
	SelectionTask(SettingContainer * setting_container);

	virtual QString caption() const;
	virtual QString prefix() const;
	virtual void startRound();
	virtual void stopRound();
	virtual void highlightCurrentBall() { _current_ball->attach(new NewWidgetAttachment(this, NewWidgetAttachment::NewBall)); };

	virtual void touchPointPressed(SceneTouchPoint * stp);
	virtual void touchPointMoved(SceneTouchPoint * stp) { Q_UNUSED(stp); }
	virtual void touchPointReleased(SceneTouchPoint * stp) { Q_UNUSED(stp); }

private:
	RigidWidgetController * _current_ball;
	QTime _acquisition_time;
	int _total_acquisition_msecs;

};

#endif // selectiontask_h__