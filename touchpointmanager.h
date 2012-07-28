
#ifndef touchpointmanager_h__
#define touchpointmanager_h__

#include "scenetouchpoint.h"
#include "tool.h"
#include "touchlistener.h"

class TouchWidgetManager;

class TouchPointManager
{
public:
	explicit TouchPointManager();
	void setSize(QSizeF size);

	// touch querying
	const QMap<int, SceneTouchPoint *> & touches() const;
	SceneTouchPoint * touch(int id) const;
	void getFreeTouches(
		QList<SceneTouchPoint *> * new_points,
		QList<SceneTouchPoint *> * existing_points = NULL) const;

	// touch list management
	void add(SceneTouchPoint * stp);
	void remove(int id);

	
	// do random stuff with stored touches
	void sendReleasedEvent(int id);
	void transferDraggingToWidget(int id, TouchWidgetController * dst);
	void transferDraggingToWidget(SceneTouchPoint * stp, TouchWidgetController * dst);
	void connectDraggingToWidget(SceneTouchPoint * stp, TouchWidgetController * dst);
	void releaseExpiredHalos(TouchWidgetManager * twm);

	// event handling
	void touchEvent( QTouchEvent * touch_event, TouchWidgetManager * twm );
	void addTouchListener(TouchListener * tl);
	void removeTouchListener(TouchListener * tl);
 
	// tool management
	//void addTool(Tool * tool) { _tools << tool; }
	void setToolList(const QList<Tool *> * tools) { _tools = tools; }
	//void setUpdateClosestWidgetsFlag(bool enabled) { _update_closest_widgets = enabled; }

protected:
	// event handling
	void touchPointPressed( SceneTouchPoint * stp, TouchWidgetManager * twm );
	void touchPointMoved( SceneTouchPoint * stp, const TouchWidgetManager * twm );
	void touchPointReleased( SceneTouchPoint * stp, const TouchWidgetManager * twm );
	//TouchWidgetController * bestAcceptingTerminatingWidget( const QList<TouchWidgetController *> accepting_widgets );

	QList<TouchWidgetController *> topmostAcceptingWidgets( const QList<TouchWidgetController *> &accepting_widgets );
	// tool management
	bool toolsAllowInteraction(const SceneTouchPoint & stp, const TouchWidgetController & twc) const;

	void notifyListeners(SceneTouchPoint * stp);
	void updateTwoClosestWidgets( SceneTouchPoint * stp, const TouchWidgetManager * twm );
	void updateAllCoveredWidgets( SceneTouchPoint * stp, const TouchWidgetManager * twm );

private:
	QList<TouchListener *> _listeners;
	QMap<int, SceneTouchPoint *> _touches;
	const QList<Tool *> * _tools;
	//bool _update_closest_widgets;
	QSizeF _parent_size;

};

#endif // touchpointmanager_h__