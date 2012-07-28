#include "selectiontask.h"
#include "variousattachments.h"

// --------------------------------------------------------
QString SelectionTask::caption() const
{
	return QString("Select %1 points (n=%2, r=%3)").arg(
		numRounds()).arg(
		scene()->settingContainer()->intProperty("num_points")).arg(
		scene()->settingContainer()->floatProperty("point_radius"));
}

// --------------------------------------------------------
SelectionTask::SelectionTask(SettingContainer * setting_container) :
	Task(setting_container)
{
	_current_ball = NULL;
	_total_acquisition_msecs = 0;
}

// --------------------------------------------------------
QString SelectionTask::prefix() const
{
	return "sel";
}

// --------------------------------------------------------
void SelectionTask::startRound()
{
	_current_ball = scene()->randomBall();
	_current_ball->setHighlighted(true);
	_current_ball->attach(new NewWidgetAttachment(_current_ball, NewWidgetAttachment::NewBall));
	_acquisition_time.start();
}

// --------------------------------------------------------
void SelectionTask::stopRound()
{
	if(_current_ball != NULL)
	{
		_current_ball->setHighlighted(false);
	}
	_current_ball = NULL;
}

// --------------------------------------------------------
void SelectionTask::touchPointPressed( SceneTouchPoint * stp )
{
	foreach(const PointWidgetConnection * pwc, stp->widgetConnections())
	{
		const RigidWidgetController * rwc = dynamic_cast<const RigidWidgetController *>(pwc->widgetController());
		if(
			rwc != NULL &&
			pwc->isDragging() // not just a bubble, its an actual press
		)
		{
			_total_acquisition_msecs += _acquisition_time.elapsed();

			float distance = 0.0f;
			if(rwc != _current_ball) // it's the right ball
			{
				distance = QLineF(_current_ball->pos(), rwc->pos()).length();
			}
			SidePaneEvent * spe = new SelectionCompletedEvent(_current_ball->pos(), _acquisition_time.elapsed(), distance);
			emit sendSidePaneEvent(spe);
			delete spe;

			SidePaneEvent * ave = new CustomSidePaneEvent(QString("Average acquisiton time: %1").arg((float) _total_acquisition_msecs / round()));
			emit sendSidePaneEvent(ave);
			delete ave;

			nextRound(rwc == _current_ball);
		}
	}
}
