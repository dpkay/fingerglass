#ifndef touchwidgetmanager_h__
#define touchwidgetmanager_h__

#include "rigidwidgetcontroller.h"
#include "touchwidgetcontroller.h"
#include "touchwidget.h"
#include "touchpointmanager.h"
#include "glresourcecontainer.h"

typedef QMultiMap<TouchWidgetType, TouchWidget *> TouchWidgetMultiMap;

class TouchWidgetManager : public QObject
{
	Q_OBJECT

public:
	TouchWidgetManager(QObject * glwidget);
	~TouchWidgetManager();
	void add(TouchWidget * tw);
	const TouchWidgetMultiMap & widgets() const;
	template<typename T> bool hasWidgetOfType() const;
	void clear();
	void remove(const TouchWidgetMultiMap::iterator & tw_it);
	void pushToFront(const TouchWidgetController * twc);
	void paintAll(GLResourceContainer * resource_container, TouchWidgetRenderer * to_texture_renderer);
	void setSelectionMode(const QString & selection_mode);
	SelectionMode selectionMode() const { return _selection_mode; }
	bool selectionModeIsDirect() const;

	// HACK
	void setCurrentBallTarget(const RigidWidgetController * ball, const RigidWidgetController * target);
	const QPair<const RigidWidgetController *, const RigidWidgetController *> * currentBallTarget() const { return _current_ball_target; }

public slots:
	void remove(TouchWidgetController * param_twc = 0);


private:
	QList<TouchWidgetType> current_types;
	TouchWidgetMultiMap touch_widgets;
	//typedef TouchWidgetPointPairSet 
	//TouchPointManager * tpm;
	//typedef QMap<QPair<RigidWidgetController *, SceneTouchPoint *>, QTime> HaloMap;
	//HaloMap _halo_rigid_widget_controllers;
	QObject * glwidget;
	SelectionMode _selection_mode;
	QPair<const RigidWidgetController *, const RigidWidgetController *> * _current_ball_target;

};


// --------------------------------------------------------
template<typename T>
bool TouchWidgetManager::hasWidgetOfType() const
{
	foreach(const TouchWidget * tw, touch_widgets)
	{
		if(dynamic_cast<const T *>(tw->controller()) != NULL)
		{
			return true;
		}
	}
	return false;
}

#endif // touchwidgetmanager_h__