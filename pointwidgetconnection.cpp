#include "pointwidgetconnection.h"
#include "rigidwidgetcontroller.h"

// --------------------------------------------------------
PointWidgetConnection::PointWidgetConnection( TouchWidgetController * twc, SceneTouchPoint * stp )
{
	this->_widget_controller = twc;
	this->_point = stp;
	this->_has_halo = false;
	this->_dragging = false;
}

// --------------------------------------------------------
TouchWidgetController * PointWidgetConnection::widgetController()
{
	return _widget_controller;
}

// --------------------------------------------------------
const SceneTouchPoint * PointWidgetConnection::point() const
{
	return _point;
}

// --------------------------------------------------------
SceneTouchPoint * PointWidgetConnection::point()
{
	return _point;
}
// --------------------------------------------------------
const TouchWidgetController * PointWidgetConnection::widgetController() const
{
	return _widget_controller;
}

// --------------------------------------------------------
PointWidgetConnection::~PointWidgetConnection()
{
	if(hasHalo())
	{
		setHalo(false);
		//delete _halo_time;
		//_halo_time = NULL;
	}
}

// --------------------------------------------------------
void PointWidgetConnection::setHalo(bool visible)
{
	RigidWidgetController * rwc = dynamic_cast<RigidWidgetController *>(_widget_controller);
	Q_ASSERT(rwc != NULL);

	rwc->setTouchPointHaloVisible(visible, point());
	_has_halo = visible;
}


// --------------------------------------------------------
bool PointWidgetConnection::hasHalo() const
{
	return _has_halo;
}

// --------------------------------------------------------
void PointWidgetConnection::setDragging( bool pressed )
{
	this->_dragging = pressed;
}

// --------------------------------------------------------
bool PointWidgetConnection::isDragging() const
{
	return _dragging;
}

// --------------------------------------------------------
bool PointWidgetConnection::isEmpty() const
{
	return !isDragging() && !hasHalo();
}