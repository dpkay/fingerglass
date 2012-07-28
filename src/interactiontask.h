#ifndef interactiontask_h__
#define interactiontask_h__

#include "task.h"
#include "variousattachments.h"

class InteractionTask : public Task
{
public:
	InteractionTask(SettingContainer * setting_container);

	QString caption() const;
	QString prefix() const;
	//void update(TouchPointManager * tpm);
	void startRound();
	void stopRound();
	virtual void highlightCurrentBall() {
		//_current_ball->attach(new NewWidgetAttachment(this, NewWidgetAttachment::NewBall));
		//_current_target->attach(new NewWidgetAttachment(this, NewWidgetAttachment::NewTarget));
	};

	void touchPointPressed(SceneTouchPoint * stp);

	void processAcquisition( const RigidWidgetController * rwc, SceneTouchPoint * stp );
	void touchPointMoved(SceneTouchPoint * stp) { Q_UNUSED(stp); }
	void touchPointReleased(SceneTouchPoint * stp);
	void createDistanceList();
	//virtual int numRounds() const;

private:
	enum TaskState { AcquisitionState, DraggingState };
	TaskState _state;

	RigidWidgetController * _current_ball;
	RigidWidgetController * _current_target;

	QPointF _current_ball_initial_pos;
	QPointF _firstpress_pos;
	QPointF _acquisition_pos;
	QTime _subtask_time;
	//float _acquisition_distance;
	int _firstpress_msecs;
	int _acquisition_msecs;
	//int _total_acquisition_msecs;
	//int _total_moving_msecs;

	int _dragging_touch_id;

	QList<float> _distances;

};

#endif // interactiontask_h__