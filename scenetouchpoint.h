#ifndef scenetouchpoint_h__
#define scenetouchpoint_h__

class SceneTouchPoint;
class RigidWidgetController;


#include <QTouchEvent>
#include <QTime>
#include <QtDebug>
#include "touchwidgetcontroller.h"
#include "pointwidgetconnection.h"

class SceneTouchPoint
{
public:
	explicit SceneTouchPoint(const QTouchEvent::TouchPoint & p, QSizeF parent_size);

	void setPoint(const QTouchEvent::TouchPoint & point);
	void untransform();
	void restart();

	// convenience
	int id() const;
	const QPointF untransformedPos() const {
		// HACK: This will not work anymore as soon as more than 1 transformation in a row happens on the same point.
		return _original_point.pos();
	}
	const QPointF untransformedStartPos() const
	{
		return _original_point.startPos();
	}
	const QPointF pos() const { return _transformed_point.pos(); }
	const QPointF startPos() const { return _transformed_point.startPos(); }
	const float radius() const;
	const QRectF rect() const { return _transformed_point.rect(); }
	void transform(const QPointF & pos, const QPointF & start_pos, const QSizeF & size);
	void transform(const QTransform & transform_);

	bool isTransformed() const { return _is_transformed; }


	Qt::TouchPointState state() const;


	void pushToFront(PointWidgetConnection * pwc);

	int age() const;
	bool isYoung() const;
	bool isFree() const;

	// widget <--> touchpoint management
	void disconnectFromWidget(TouchWidgetController * owner, bool mutual = true);
	PointWidgetConnection * connectToWidget(TouchWidgetController * owner, bool mutual = true);
	const QList<PointWidgetConnection *> & widgetConnections() const;
	PointWidgetConnection * widgetConnection( TouchWidgetController * widget_controller );
	PointWidgetConnection * bottomWidgetConnection();
	const PointWidgetConnection * bottomWidgetConnection() const;
	bool hasWidgetConnection(const TouchWidgetController * twc) const;


	int numPressedWidgets() const;
	QList<TouchWidgetController *> pressedTouchWidgetControllers();

	const QList<RigidWidgetController *> & closestRigidWidgets() const;
	QList<RigidWidgetController *> & closestRigidWidgets();

	QRectF bubbleRect() const;

	bool visibleInScene() const { return _visible_in_scene; }
	void setVisibleInScene(bool visible) { _visible_in_scene = visible; }

	enum VisibilityInMagnification
	{
		NeverVisible,
		VisibleOnlyWithAreaCursor,
		AlwaysVisible,
	};

	void setVisiblityInMagnification(const VisibilityInMagnification & visible) { _visibility_in_magnification = visible; }
	const VisibilityInMagnification & visibilityInMagnification() const { return _visibility_in_magnification; }


protected:
	//const QTouchEvent::TouchPoint * point() const;

	//QTouchEvent::TouchPoint * point();
	QPointF normalizedToParent(const QPointF & p);


private:
	//TouchWidgetController * _owner;
	QTime _age;
	QTouchEvent::TouchPoint _original_point;
	QTouchEvent::TouchPoint _transformed_point;
	QList<PointWidgetConnection *> _widget_connections;
	QList<RigidWidgetController *> _closest_rigid_widgets;
	bool _is_transformed;
	QSizeF _parent_size;
	VisibilityInMagnification _visibility_in_magnification;
	bool _restarted;
	QPointF _old_start_pos;
	bool _visible_in_scene;

};

#endif // ownedtouchpoint_h__