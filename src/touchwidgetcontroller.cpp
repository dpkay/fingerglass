#include "touchwidgetcontroller.h"

// --------------------------------------------------------
const QMap<int, PointWidgetConnection *> & TouchWidgetController::touchPointConnections() const
{
	return _touch_point_connections;
}

// --------------------------------------------------------
void TouchWidgetController::close()
{
	emit closed();
}

// --------------------------------------------------------
PointWidgetConnection * TouchWidgetController::connectToTouchPoint( SceneTouchPoint * stp, bool mutual )
{
	Q_ASSERT(!touchPointConnections().contains(stp->id()));

	PointWidgetConnection * pwc = stp->widgetConnection(this);
	if(pwc == NULL)
	{
		pwc = new PointWidgetConnection(this, stp);
	}
	_touch_point_connections[stp->id()] = pwc;

	Q_ASSERT(touchPointConnection(stp) != NULL);

	if(mutual)
	{
		stp->connectToWidget(this, false);
	}
	Q_ASSERT(stp->widgetConnection(this) == pwc);

	return pwc;
}

// --------------------------------------------------------
void TouchWidgetController::disconnectFromTouchPoint( SceneTouchPoint * stp, bool mutual )
{
	Q_ASSERT(touchPointConnections().contains(stp->id()));
	PointWidgetConnection * pwc = _touch_point_connections.value(stp->id());
	_touch_point_connections.remove(stp->id());

	if(mutual)
	{
		stp->disconnectFromWidget(this, false);
		delete pwc;
	}
}

// --------------------------------------------------------
PointWidgetConnection * TouchWidgetController::touchPointConnection( SceneTouchPoint * stp )
{
	foreach(PointWidgetConnection * pwc, _touch_point_connections)
	{
		if(pwc->point() == stp)
		{
			return pwc;
		}
	}
	return NULL;
}

// --------------------------------------------------------
PointWidgetConnection * TouchWidgetController::touchPointConnection( int point_id )
{
	foreach(PointWidgetConnection * pwc, _touch_point_connections)
	{
		if(pwc->point()->id() == point_id)
		{
			return pwc;
		}
	}
	return NULL;
}

// --------------------------------------------------------
int TouchWidgetController::numPressedTouchPoints() const
{
	int count = 0;
	foreach(PointWidgetConnection * pwc, _touch_point_connections)
	{
		if(pwc->isDragging())
		{
			++count;
		}
	}
	return count;
}

// --------------------------------------------------------
TouchWidgetType TouchWidgetController::type() const
{
	return _type;
}

// --------------------------------------------------------
TouchWidgetController::TouchWidgetController( TouchWidgetType type )
{
	_type = type;
	_closing_timer = NULL;
}

// --------------------------------------------------------
void TouchWidgetController::startClosingTimer(int closing_delay)
{
	Q_ASSERT(_closing_timer == NULL);
	this->_closing_delay = closing_delay;
	this->_closing_timer = new QTime;
	this->_closing_timer->start();
	this->_closing_timer_needs_unsubscription = !TimeSingleton::instance()->isSubscribed(this);
	TimeSingleton::instance()->subscribe(this);
	Q_ASSERT(debug_closingTimerConsistent());
}

// --------------------------------------------------------
void TouchWidgetController::stopClosingTimer()
{
	Q_ASSERT(_closing_timer != NULL);
	delete _closing_timer;
	this->_closing_timer = NULL;
	if(this->_closing_timer_needs_unsubscription)
	{
		TimeSingleton::instance()->unsubscribe(this);
	}
	Q_ASSERT(debug_closingTimerConsistent());
}

// --------------------------------------------------------
TouchWidgetController::~TouchWidgetController()
{
	if(this->_closing_timer != NULL)
	{
		delete this->_closing_timer;
	}
	if(TimeSingleton::instance()->isSubscribed(this))
	{
		TimeSingleton::instance()->unsubscribe(this);
	}
	foreach(WidgetAttachment * wa, _attachments)
	{
		delete wa;
	}
}

// --------------------------------------------------------
bool TouchWidgetController::debug_closingTimerConsistent()
{
	// if the closing timer is not NULL, then we certainly have to be subscribed
	return (this->_closing_timer==NULL) || TimeSingleton::instance()->isSubscribed(this);
}

// --------------------------------------------------------
void TouchWidgetController::timeStep()
{
	if(isClosingTimerRunning())
	{
		if(this->_closing_timer->elapsed() >= _closing_delay)
		{
			close();
		}
	}
}

// --------------------------------------------------------
PointWidgetConnection * TouchWidgetController::singlePressedTouchPointConnection() const
{
	Q_ASSERT(numPressedTouchPoints()==1);
	foreach(PointWidgetConnection * pwc, _touch_point_connections)
	{
		if(pwc->isDragging())
		{
			return pwc;
		}
	}
	Q_ASSERT(false);
	return NULL;
}
// --------------------------------------------------------
//void TouchWidgetController::transformTouches( QMap<int, TouchPoint> * touches ) const
//{
//	// do nothing by default
//}