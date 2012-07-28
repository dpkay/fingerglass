#ifndef handlewidgetcontroller_h__
#define handlewidgetcontroller_h__

#include "settingcontainer.h"
#include "touchwidgetcontroller.h"
#include "rigidwidgetcontroller.h"
#include "handle.h"
#include "magnifyingglass.h"
#include "timesingleton.h"
#include "magnifyingglassoptimizer.h"

class Handle;

class HandleWidgetController : public TouchWidgetController
{
	Q_OBJECT
public:
	HandleWidgetController(RigidWidgetController * attached_widget,
		const SettingContainer & tool_setting_container, const MagnifyingGlass & mg);
	~HandleWidgetController();

	// touch event handling
	void touchPointReleased( SceneTouchPoint * p );
	void touchPointMoved( SceneTouchPoint * p );
	void touchPointPressed( SceneTouchPoint * p );
	bool acceptTouchPoint(const SceneTouchPoint & p) const;
	bool forwardTouchPoint(const SceneTouchPoint & p) const;
	void createClutchHandle(const QPointF & pos, const MagnifyingGlass * mg);
	void correctWidgetPosition();
	const RigidWidgetController * attachedWidget() const;
	const QList<const Handle *> handles() const;
	const AbsoluteHandle * absoluteHandle() const { return _absolute_handle; }
	const ClutchHandle * clutchHandle() const { return _clutch_handle; }
	const SceneTouchPoint * clutchResizingTouchPoint() const;
	const bool showHandle() const; 
	bool isLocked() const { return _lock_touch != NULL; }
	QPair<SceneTouchPoint *, SceneTouchPoint *> absoluteTouches();
	enum AbsoluteHandleMode
	{
		NoAbsoluteHandle,
		OneFingerHandle,
		TwoFingerHandle,
		VirtualTwoFingerHandle
	};
	AbsoluteHandleMode absoluteHandleMode() const;
	SceneTouchPoint * clutchTouch() { return _clutch_touch; }

protected slots:
	void timeStep();
	void updateAttachedWidget();

signals:
	void createMagnifyingWidget();

protected:

	Handle * handleByTouchId(int id) const;
	QPointF cdScaledTouchDelta( const SceneTouchPoint * p ) const;
	bool acceptAsResizer(const SceneTouchPoint & p) const;
	bool acceptAsAbsoluteTouch(const SceneTouchPoint & p) const;
	bool acceptAsLock(const SceneTouchPoint & p) const;

	void updateAbsoluteHandle();
	void updateClutchHandle();

	void startClosing();
	void setupNewAbsoluteHandleMode();
	void resetTouchesAndWidgets();
	void allocateTouch( SceneTouchPoint * p );
	void deallocateTouch( SceneTouchPoint * p );


private:
	QPointF * _virtual_second_touch;
	AbsoluteHandle * _absolute_handle;
	ClutchHandle * _clutch_handle;
	SceneTouchPoint * _absolute_touch;
	SceneTouchPoint * _absolute_secondary_touch;
	SceneTouchPoint * _clutch_touch;
	SceneTouchPoint * _lock_touch;
	RigidWidgetController * _attached_widget;
	//QPointF * _attached_widget_original_pos;
	const SettingContainer & _tool_setting_container;
	SceneTouchPoint * _clutch_resizing_touch_point;
	QTimer timer;

};

#endif // handlewidgetcontroller_h__