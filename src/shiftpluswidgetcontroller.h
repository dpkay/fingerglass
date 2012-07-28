#ifndef shiftpluswidgetcontroller_h__
#define shiftpluswidgetcontroller_h__

#include "magnifyingglassmanager.h"
#include "singlehandedprecisionwidgetcontroller.h"

class ShiftPlusWidgetController : public SingleHandedPrecisionWidgetController
{
	Q_OBJECT
public:
	ShiftPlusWidgetController(const SettingContainer & tool_setting_container);
	~ShiftPlusWidgetController();

	virtual bool acceptTouchPoint(const SceneTouchPoint & p) const;

	const MagnifyingGlass * magnifyingGlass() const { Q_ASSERT(_mgm != NULL); return _mgm->singleMagnifyingGlass(); }

	virtual QPointF gripPoint() const { return _mgm->baseGlass()->dstCenterT(); }
	virtual QPointF tipPoint() const { return _mgm->singleMagnifyingGlass()->srcCenterT(); }

	//virtual void touchPointMoved( SceneTouchPoint * p ) { SingleHandedPrecisionWidgetController::touchPointMoved(p); updateTimeout(); }

	const QSizeF & gripSize() const;

public slots:
	void updateTimeout();

protected:
	// from SingleHandedPrecisionWidgetController
	virtual void initializeMagnifyingGlass(SceneTouchPoint * stp);
	virtual void updateToolMode();
	virtual void updateAndTransformPoints();
	virtual void resizeGlass();
	virtual void updateAfterMovement();

	void updateDesiredAngle();
	virtual void handleNewlyAttachedWidget(RigidWidgetController * rwc);
	virtual void startClosingAnimation(RigidWidgetController * rwc);
	virtual const SceneTouchPoint * resizingTouch() const;
	virtual QPointF map(const QPointF & p) const;
	//virtual QPointF map(const QPointF & p) const { return p; }

	virtual void setGripPoint(const QPointF & p) { _mgm->setDesiredBaseGlassCenter(p); }
	virtual void setTipPoint(const QPointF & p) { _mgm->setSrcPos(p); }

	virtual void test(SceneTouchPoint * p);

	// other
	//void updateMagnifyingGlasses();

	virtual float ratio() const { return _mgm->singleMagnifyingGlass()->ratio(); }

private:
	//QPointF _grip_point;
	QTimer * _update_timer;
	MagnifyingGlassManager * _mgm;
	QTransform * _transform;
	RelativeValueChanger<float> * _relative_resizer;
	bool _request_update;
};


#endif // precisionhandlewidgetcontroller_h__