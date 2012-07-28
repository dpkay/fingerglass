#include "interactiontask.h"
#include "variousattachments.h"
#include "magnifyingwidgetcontroller.h"
#include "rigidwidgetcontroller.h"

// --------------------------------------------------------
QString InteractionTask::caption() const
{
	//return "Select 50 points!";
	return QString("Move %1 points (n=%2, r=%3)").arg(
		numRounds()).arg(
		scene()->settingContainer()->intProperty("num_points")).arg(
		scene()->settingContainer()->floatProperty("point_radius"));
}

// --------------------------------------------------------
InteractionTask::InteractionTask(SettingContainer * setting_container) :
	Task(setting_container)
{
	_current_ball = NULL;
	_current_target = NULL;
	createDistanceList();
}

// --------------------------------------------------------`
QString InteractionTask::prefix() const
{
	return "int";
}

// --------------------------------------------------------
void InteractionTask::startRound()
{
	_firstpress_msecs = -1;
	_acquisition_msecs = 0;

	//_current_ball = scene()->randomBall();
	//_current_ball->setHighlighted(true);
	//_current_ball->attach(new NewWidgetAttachment(_current_ball, NewWidgetAttachment::NewBall));


	//// create new target
	//Q_ASSERT(_current_target == NULL);
	//_current_target = scene()->createTarget(_current_ball->pos());

	scene()->createBallTargetPair(&_current_ball, &_current_target, _distances.value(round()-1));
	_current_ball->setHighlighted(true);

	if(scene()->settingContainer()->boolProperty("highlight_targets"))
	{
		_current_ball->attach(new NewWidgetAttachment(_current_ball, NewWidgetAttachment::NewBall));
		_current_target->attach(new NewWidgetAttachment(_current_target, NewWidgetAttachment::NewTarget));
	}

	_current_ball_initial_pos = _current_ball->pos();
	_subtask_time.start();
	_state = AcquisitionState;
}

// --------------------------------------------------------
void InteractionTask::stopRound()
{
	// kill ball
	if(_current_ball != NULL)
	{
		_current_ball->setHighlighted(false);
		_current_ball = NULL;
	}

	// kill target
	if(_current_target != NULL)
	{
		scene()->remove(_current_target);
		//delete _current_target;
		_current_target = NULL;
	}
}

// --------------------------------------------------------
void InteractionTask::touchPointPressed( SceneTouchPoint * stp )
{
	// HACK
	const_cast<TouchWidgetManager *>(scene()->touchWidgetManager())->setCurrentBallTarget(NULL, NULL);
	if(_firstpress_msecs == -1)
	{
		if(!scene()->touchWidgetManager()->selectionModeIsDirect())
		{
			_firstpress_pos = stp->untransformedPos();
			_firstpress_msecs = _subtask_time.restart();
			taskStatistics()->addStart(_firstpress_msecs);
		}
		else
		{
			_firstpress_msecs = 0;
			_subtask_time.start();
		}
	}
	else
	if(_state == AcquisitionState)
	{
		foreach(const PointWidgetConnection * pwc, stp->widgetConnections())
		{
			const RigidWidgetController * rwc = dynamic_cast<const RigidWidgetController *>(pwc->widgetController());
			if( rwc != NULL &&
				stp->state() == Qt::TouchPointPressed && // just happened right now
				pwc->isDragging() && // not just a bubble, its an actual press
				_current_ball != NULL // we're not done yet
				)
			{
				//qDebug() << "checking stuff";
				processAcquisition(rwc, stp);
			}
		}
	}
}

// --------------------------------------------------------
void InteractionTask::touchPointReleased( SceneTouchPoint * stp )
{
	if(_state == AcquisitionState && _current_ball != NULL)
	{
		foreach(const PointWidgetConnection * pwc, stp->widgetConnections())
		{
			const MagnifyingWidgetController * mwc = dynamic_cast<const MagnifyingWidgetController *>(pwc->widgetController());
			if( mwc != NULL &&
				pwc->isDragging() &&
				//mwc->isInteriorPoint(stp))
				mwc->isSpanningPoint(*stp))
			{
				foreach(const PointWidgetConnection * pwc, mwc->touchPointConnections())
				{
					const RigidWidgetController * rwc = dynamic_cast<const RigidWidgetController *>(pwc->point()->bottomWidgetConnection()->widgetController());
					if( rwc != NULL && pwc->isDragging() )
					{
						processAcquisition(rwc, stp);
					}
				}
			}
		}
	}
	else
	if(_state == DraggingState && stp->id() == _dragging_touch_id)
	{
		Q_ASSERT(_current_target != NULL);

		float moving_distance = QLineF(_current_ball->pos(), _current_target->pos()).length();
		float moving_msecs = _subtask_time.elapsed();
		//_total_moving_msecs += _moving_msecs;

		InteractionResult result;
		if(moving_distance < _current_ball->coreRadius() + _current_target->coreRadius())
		{
			result = Success;
		}
		else
		{
			result = DraggingFailed;
		}
		taskStatistics()->addDrag(moving_msecs, result==Success);

		SidePaneEvent * spe = new InteractionCompletedEvent(
			_current_ball_initial_pos,
			_current_target->pos(),
			_firstpress_pos,
			_acquisition_pos,
			stp->untransformedPos(),
			_firstpress_msecs,
			_acquisition_msecs,
			moving_msecs,
			result
		);
		emit sendSidePaneEvent(spe);
		delete spe;

		nextRound(result == DraggingFailed);
	}
}

// --------------------------------------------------------
void InteractionTask::createDistanceList()
{
	Q_ASSERT(_distances.empty());
	const int distances = settingContainer()->intProperty("num_distances");
	const int blocks = settingContainer()->intProperty("num_blocks");
	const int min_exponent = settingContainer()->intProperty("min_exponent");

	for(int block=0; block<blocks; ++block)
	{
		typedef QPair<int, int> IntPair;
		QList<IntPair> block_distances;
		for(int i=0; i<distances; ++i)
		{
			block_distances << qMakePair(qrand(), i + min_exponent);
		}
		qSort(block_distances);
		foreach(IntPair dp, block_distances)
		{
			_distances << dp.second;
		}
	}
}

// --------------------------------------------------------
void InteractionTask::processAcquisition( const RigidWidgetController * rwc, SceneTouchPoint * stp )
{
	_acquisition_msecs = _subtask_time.restart();
	//_total_acquisition_msecs += _acquisition_msecs;

	//_acquisition_distance = 0.0f;
	if(rwc == _current_ball) // it's the right ball
	{
		// successful acquisition
		_acquisition_pos = stp->untransformedPos();
		_state = DraggingState;
		//_dragging_touch_id = stp->id();
		Q_ASSERT(rwc->touchPointConnections().size() == 1);
		_dragging_touch_id = rwc->touchPointConnections().begin().key();
		qDebug() << "state changed to DraggingState";
		taskStatistics()->addAcquisition(_acquisition_msecs, true);
		if(scene()->settingContainer()->boolProperty("highlight_targets"))
		{
			_current_target->attach(new NewWidgetAttachment(_current_target, NewWidgetAttachment::NewTarget));
		}
	}
	else
	{
		// failed acquisition
		SidePaneEvent * spe = new InteractionCompletedEvent(
			_current_ball->pos(),
			QPointF(),          // destination pos
			_firstpress_pos,
			stp->untransformedPos(),         // acquisition pos
			QPointF(),          // released pos
			_firstpress_msecs,
			_acquisition_msecs,
			0,                  // dragging time
			AcquisitionFailed  
			);
		taskStatistics()->addAcquisition(_acquisition_msecs, false);
		emit sendSidePaneEvent(spe);
		delete spe;

		nextRound(true);
	}
}
//// --------------------------------------------------------
//int InteractionTask::numRounds() const
//{
//	return settingContainer()->intProperty("num_distances") * settingContainer()->intProperty("num_rounds");
//}