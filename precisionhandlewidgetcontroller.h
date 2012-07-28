#ifndef precisionhandlewidgetcontroller_h__
#define precisionhandlewidgetcontroller_h__

#include "magnifyingglass.h"
#include "singlehandedprecisionwidgetcontroller.h"

class PrecisionHandleWidgetController : public SingleHandedPrecisionWidgetController
{
public:
	PrecisionHandleWidgetController(const SettingContainer & tool_setting_container);
	~PrecisionHandleWidgetController();

	virtual bool acceptTouchPoint(const SceneTouchPoint & p) const;
	float tipRadius() const { return _mg->srcRadiusT(); }
	const MagnifyingGlass * gripMagnifyingGlass() const { Q_ASSERT(_mg != NULL); return _mg; }

	virtual QPointF gripPoint() const { return _mg->dstCenterT(); }
	virtual QPointF tipPoint() const { return _mg->srcCenterT(); }

protected:
	//virtual void adjustTipToAttachedWidget();
	virtual void updateToolMode();
	virtual void updateAndTransformPoints();
	virtual void resizeGlass();
	virtual void updateAfterMovement();
	virtual void initializeMagnifyingGlass(SceneTouchPoint * stp);
	virtual void startClosingAnimation(RigidWidgetController * rwc);
	virtual const SceneTouchPoint * resizingTouch() const;
	virtual QPointF map(const QPointF & p) const { Q_ASSERT(_transform != NULL); return _transform->map(p); }
	virtual void test(SceneTouchPoint * p);

	virtual void setGripPoint(const QPointF & p) { _mg->setDstCenter(p); }
	virtual void setTipPoint(const QPointF & p) { _mg->setSrcCenter(p); }

	virtual float ratio() const { return _mg->ratio(); }

	float desiredMagnifyingGlassRadius() const;

private:
	MagnifyingGlass * _mg;
	QTransform * _transform;
	RelativeValueChanger<float> * _relative_resizer;
	bool _initial_phase;

};


#endif // precisionhandlewidgetcontroller_h__