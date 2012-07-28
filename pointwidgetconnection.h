#ifndef pointwidgetconnection_h__
#define pointwidgetconnection_h__

class PointWidgetConnection;

#include "touchwidgetcontroller.h"
#include "scenetouchpoint.h"

class PointWidgetConnection
{
public:
	PointWidgetConnection(TouchWidgetController * twc, SceneTouchPoint * stp);
	~PointWidgetConnection();
	const TouchWidgetController * widgetController() const;
	TouchWidgetController * widgetController();
	const SceneTouchPoint * point() const;
	SceneTouchPoint * point();
	bool hasHalo() const;
	void setHalo(bool visible);
	void setDragging(bool pressed);
	bool isDragging() const;
	bool isEmpty() const;

private:
	TouchWidgetController * _widget_controller;
	SceneTouchPoint * _point;
	bool _has_halo;
	bool _dragging;

};

#endif // pointwidgetconnection_h__