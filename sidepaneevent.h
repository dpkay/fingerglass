#ifndef sidepaneevent_h__
#define sidepaneevent_h__

#include "taskstatistics.h"

class SidePaneEvent
{
public:
	virtual ~SidePaneEvent() {}

	QString timedScreenLogString() const
	{
		QString current_time = QTime::currentTime().toString("hh:mm:ss");
		return QString("[%1] %2").arg(current_time).arg(screenLogString());
	}

protected:
	virtual QString screenLogString() const = 0;

};


class CustomSidePaneEvent : public SidePaneEvent
{
public:
	CustomSidePaneEvent(const QString & log_string)
	{
		_log_string = log_string;
	}

	virtual QString screenLogString() const { return _log_string; }

private:
	QString _log_string;

};


class TaskCreationEvent : public SidePaneEvent
{
public:
	TaskCreationEvent(const QString & caption, const QString & prefix, const TaskStatistics * task_statistics)
	{
		this->_caption = caption;
		this->_prefix = prefix;
		this->_task_statistics = task_statistics;
	}

	const QString & prefix() { return this->_prefix; }
	const TaskStatistics * taskStatistics() const
	{
		return _task_statistics;
	}

protected:
	virtual QString screenLogString() const
	{
		return QString("New task: %1").arg(_caption);
	}

private:
	QString _caption;
	QString _prefix;
	const TaskStatistics * _task_statistics;
};


class TaskCompletionEvent : public SidePaneEvent
{
protected:

	virtual QString screenLogString() const
	{
		return "Task completed.";
	}
};

class RoundCompletedEvent : public SidePaneEvent
{
public:
	virtual QString fileLogString() const = 0;
};

class SelectionCompletedEvent : public RoundCompletedEvent
{
public:
	SelectionCompletedEvent(QPointF pos, int elapsed, float distance) //:
	{
		this->_pos = pos;
		this->_elapsed = elapsed;
		this->_distance = distance;
	}

	const QPointF & pos() const { return _pos; }
	const int elapsed() const { return _elapsed; }
	const float distance() const { return _distance; }

protected:
	virtual QString screenLogString() const
	{
		return QString("%1 selection in %2 msec").arg(_distance==0.0f ? "Successful" : "Wrong").arg(_elapsed);
	}

	virtual QString fileLogString() const
	{
		return QString("%1 %2 %3 %4").
			arg(_pos.x()).
			arg(_pos.y()).
			arg(_elapsed).
			arg(_distance);
	}

private:
	QPointF _pos;
	int _elapsed;
	float _distance;
};

enum InteractionResult
{
	AcquisitionFailed,
	DraggingFailed,
	Success
};


class InteractionCompletedEvent : public RoundCompletedEvent
{
public:
	InteractionCompletedEvent(
		QPointF ball_pos, QPointF target_pos,
		QPointF first_press_pos, QPointF acquisition_pos, QPointF release_pos,
		int first_press_elapsed, int acquisition_elapsed, int dragging_elapsed,
		InteractionResult result
	)
	{
		this->_ball_pos = ball_pos;
		this->_target_pos = target_pos;
		this->_first_press_pos = first_press_pos;
		this->_acquisition_pos = acquisition_pos;
		this->_release_pos = release_pos;
		this->_first_press_elapsed = first_press_elapsed;
		this->_acquisition_elapsed = acquisition_elapsed;
		this->_dragging_elapsed = dragging_elapsed;
		this->_result = result;
	}

	virtual QString screenLogString() const
	{
		if(_first_press_elapsed == 0)
		{
			switch(_result)
			{
			case AcquisitionFailed:
				return QString("Acquisition failed after %1 msec.").arg(_acquisition_elapsed);
			case DraggingFailed:
				return QString("Dragging failed after %1/%2 msec.").arg(_acquisition_elapsed).arg(_dragging_elapsed);
			case Success:
				return QString("Successful after %1/%2 msec.").arg(_acquisition_elapsed).arg(_dragging_elapsed);
			default:
				return "ERROR";
			}
		}
		else
		{
			switch(_result)
			{
			case AcquisitionFailed:
				return QString("Acquisition failed: %1/%2 ms.").arg(_first_press_elapsed).arg(_acquisition_elapsed);
			case DraggingFailed:
				return QString("Drag failed: %1/%2/%3 ms.").arg(_first_press_elapsed).arg(_acquisition_elapsed).arg(_dragging_elapsed);
			case Success:
				return QString("Success: %1/%2/%3 ms.").arg(_first_press_elapsed).arg(_acquisition_elapsed).arg(_dragging_elapsed);
			default:
				return "ERROR";
			}
		}
	}

	virtual QString fileLogString() const
	{
		return QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14").
			arg(_ball_pos.x()).
			arg(_ball_pos.y()).
			arg(_target_pos.x()).
			arg(_target_pos.y()).
			arg(_first_press_pos.x()).
			arg(_first_press_pos.y()).
			arg(_acquisition_pos.x()).
			arg(_acquisition_pos.y()).
			arg(_release_pos.x()).
			arg(_release_pos.y()).
			arg(_first_press_elapsed).
			arg(_acquisition_elapsed).
			arg(_dragging_elapsed).
			arg(static_cast<int>(_result));
	}

private:
	QPointF _ball_pos;
	QPointF _target_pos;
	QPointF _first_press_pos;
	QPointF _acquisition_pos;
	QPointF _release_pos;
	int _first_press_elapsed;
	int _acquisition_elapsed;
	int _dragging_elapsed;
	InteractionResult _result;
};

class StatisticReportEvent : SidePaneEvent
{

};


#endif // sidepaneevent_h__