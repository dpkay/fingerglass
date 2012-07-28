#ifndef touchwidgetcontroller_h__
#define touchwidgetcontroller_h__

class TouchWidgetController;

#include <QTouchEvent>
#include "scenetouchpoint.h"
#include "pointwidgetconnection.h"
#include "timesingleton.h"
#include "timed.h"
#include "widgetattachment.h"

enum TouchWidgetType
{
	NullType,
	MapWidgetType,
	TargetWidgetType,
	WaypointWidgetType,
	HandleWidgetType,
	TouchDisplayWidgetType,
	MagnifyingWidgetType
};

class TouchWidgetController : public QObject, public Timed
{
	Q_OBJECT

public:
	TouchWidgetController(TouchWidgetType type);
	~TouchWidgetController();

	typedef QTouchEvent::TouchPoint TouchPoint;

	// touch event handling.
	virtual void touchPointPressed(SceneTouchPoint * p) = 0;
	virtual void touchPointMoved(SceneTouchPoint * p) = 0;
	virtual void touchPointReleased(SceneTouchPoint * p) = 0;
	virtual bool acceptTouchPoint(const SceneTouchPoint & p) const = 0;
	virtual bool forwardTouchPoint(const SceneTouchPoint & p) const = 0;

	// widget <--> touchpoint management
	PointWidgetConnection * connectToTouchPoint(SceneTouchPoint * p, bool mutual = true);
	void disconnectFromTouchPoint(SceneTouchPoint * p, bool mutual = true);
	const QMap<int, PointWidgetConnection *> & touchPointConnections() const;
	PointWidgetConnection * touchPointConnection( SceneTouchPoint * stp );
	PointWidgetConnection * touchPointConnection( int point_id );
	PointWidgetConnection * singlePressedTouchPointConnection() const;

	int numPressedTouchPoints() const;
	TouchWidgetType type() const;

	// for stuff like magnifying glasses
	//virtual void transformTouches(QMap<int, TouchPoint> * touches) const;

	// from Timed
	virtual void timeStep();
	const QList<WidgetAttachment *> & attachments() const { return _attachments; };
	void attach(WidgetAttachment * wa) {
		_attachments << wa;
		connect(wa, SIGNAL(close()), this, SLOT(detach()));
	}

public slots:
	void detach()
	{
		WidgetAttachment * sender_attachment = dynamic_cast<WidgetAttachment *>(QObject::sender());
		Q_ASSERT(sender_attachment != NULL);
		_attachments.removeOne(sender_attachment);
		delete sender_attachment;
		Q_ASSERT(!_attachments.contains(sender_attachment));
	}

signals:
	void closed();
	//void registerTouchForward(int id);

protected:
	void close();
	void startClosingTimer(int closing_delay);
	void stopClosingTimer();
	bool isClosingTimerRunning() { return _closing_timer != NULL; }
	bool debug_closingTimerConsistent();

private:
	QTime * _closing_timer;
	int _closing_delay;
	bool _closing_timer_needs_unsubscription;
	QMap<int, PointWidgetConnection *> _touch_point_connections;
	TouchWidgetType _type;
	QList<WidgetAttachment *> _attachments;
};

#endif // touchwidget_h__