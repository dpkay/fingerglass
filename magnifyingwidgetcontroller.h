#ifndef magnifyingwidgetcontroller_h__
#define magnifyingwidgetcontroller_h__

#include "rigidwidgetcontroller.h"
#include "touchwidgetcontroller.h"
#include "magnifyingglassmanager.h"
#include "settingcontainer.h"
#include <cmath>

class MagnifyingWidgetController : public TouchWidgetController
{
	Q_OBJECT

public:
	MagnifyingWidgetController(
		const SettingContainer & tool_setting_container,
		MagnifyingGlass * zoom_glass = NULL
	);
	~MagnifyingWidgetController();

	// touch event handling
	void touchPointReleased( SceneTouchPoint * p );
	void touchPointMoved( SceneTouchPoint * p );
	void touchPointPressed( SceneTouchPoint * p);
	void touchPointPressed( SceneTouchPoint * p, bool force_glass );
	bool acceptTouchPoint(const SceneTouchPoint & p) const;
	bool forwardTouchPoint(const SceneTouchPoint & p) const;

	// drawing related stuff
	const QVector<MagnifyingGlass *> & magnifyingGlasses() const;
	void updateMagnifyingGlasses();

	// specific widget settings
	void addSpanningPoint(SceneTouchPoint * p);
	void removeSpanningPoint(SceneTouchPoint * p);
	QList<int> innerTouchIds() const;
	bool detached();
	//float cdRatioOfPoint(int id) const;

	bool isInteriorPoint(const SceneTouchPoint * p) const;
	bool isSpanningPoint(const SceneTouchPoint & p) const;

	bool locked() const { return _locking_touch != NULL; }
	// timed stuff
	void timeStep();
	//const MagnifyingGlass * singleMagnifyingGlass();
	const MagnifyingGlass * magnifyingGlassOfPoint(SceneTouchPoint * p);
	const SceneTouchPoint * resizingTouchPoint() const { return _dst_resizing_touch; }

	float magnifyingGlassDistance() const;

	void enqueuePole(const QPointF * p) { _enqueued_pole = p; }

signals:
	void createHandleWidgetIfNecessary();

protected:
	int numSpanningPoints() const;
	QPointF virtualSpanningPointPos() const;
	void updateSpanningDelta();
	QPair<QPointF, QPointF> spanningPositions() const;
	float spanningDiameter() const;
	void transformTouchPoint(SceneTouchPoint * p);
	bool acceptAsSecondSpanningPoint(const SceneTouchPoint & p) const;
	bool isYoung() const;
	bool showGlass() const;
	bool isOnBaseGlassSide( const SceneTouchPoint &p ) const;

private:
	MagnifyingGlassManager * _mgm;
	QPair<SceneTouchPoint *, SceneTouchPoint *> spanning_points;
	QMap<int, const MagnifyingGlass *> _owning_magnifying_glasses;
	SceneTouchPoint * _dst_resizing_touch;
	SceneTouchPoint * _locking_touch;
	bool request_update;
	QPointF pos_delta;
	const SettingContainer & _tool_setting_container;
	QTime _age;
	QMap<int, QPointF> _start_positions;
	bool _just_resized;
	QList<RigidWidgetController *> _active_halos;
	MagnifyingGlass * _initial_zoom_glass;

	friend class MagnifyingWidgetRenderer;

	bool _force_detached;
	float _desired_ratio;

	const QPointF * _enqueued_pole;
};

#endif // magnifyingwidget_h__