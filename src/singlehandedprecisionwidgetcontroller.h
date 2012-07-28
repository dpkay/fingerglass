#ifndef singlehandedprecisionwidgetcontroller_h__
#define singlehandedprecisionwidgetcontroller_h__

#include "touchwidgetcontroller.h"
#include "settingcontainer.h"
#include "magnifyingglass.h"

class SingleHandedPrecisionWidgetController : public TouchWidgetController
{
public:
	SingleHandedPrecisionWidgetController(const SettingContainer & tool_setting_container);
	~SingleHandedPrecisionWidgetController();

	// events
	virtual void touchPointPressed(SceneTouchPoint * p);
	virtual void touchPointMoved( SceneTouchPoint * p );
	virtual void touchPointReleased(SceneTouchPoint * p);
	virtual bool forwardTouchPoint( const SceneTouchPoint & p ) const;

	// definition
	void setDefiningTouch(SceneTouchPoint * stp);
	bool hasDefiningTouch() const { return _defining_touch != NULL; }

	// access
	virtual QPointF gripPoint() const = 0;
	virtual QPointF tipPoint() const = 0;

protected:
	virtual void initializeMagnifyingGlass(SceneTouchPoint * stp) = 0;
	virtual void updateToolMode() = 0;
	virtual void updateAndTransformPoints() = 0;
	void adjustTipToAttachedWidget();
	virtual void handleNewlyAttachedWidget(RigidWidgetController * rwc) { Q_UNUSED(rwc); }
	virtual void resizeGlass() = 0;
	virtual void updateAfterMovement() = 0;
	virtual void startClosingAnimation(RigidWidgetController * rwc) = 0;//{ Q_UNUSED(rwc); }
	virtual const SceneTouchPoint * resizingTouch() const = 0;
	virtual QPointF map(const QPointF & p) const = 0;
	virtual void test(SceneTouchPoint * p) = 0;

	struct DefiningTouch
	{
		explicit DefiningTouch(SceneTouchPoint * stp_)
		{
			Q_ASSERT(stp!=NULL);
			stp=stp_;
			transformed_start_pos=NULL;
		}
		~DefiningTouch()
		{
			delete transformed_start_pos;
		}
		SceneTouchPoint * stp;
		QPointF * transformed_start_pos;
	};

	template <typename T> struct RelativeValueChanger
	{
		RelativeValueChanger(const T & value, const DefiningTouch * dt)
		{
			start_value = value;
			start_defining_point_pos = dt->stp->untransformedPos();
		}
		//changer = new RelativeValueChanger;
		//changer->start_value = _ratio;
		//changer->start_defining_point_pos = definingTouch()->stp->untransformedPos();

		T start_value;
		QPointF start_defining_point_pos;
	};

	// touch management
	const DefiningTouch * definingTouch() const { return _defining_touch; }
	void createDefiningTouch(SceneTouchPoint * stp);
	void deleteDefiningTouch();
	void createTransformedStartPos(const QPointF & pos);
	const QList<SceneTouchPoint *> additionalPoints() const { return _additional_points; }

	// point management
	//void setGripPoint(const QPointF & p) { _grip_point = p; }
	//void setTipPoint(const QPointF & p) { _tip_point = p; }

	// resizing
	float desiredRelativeChangingValue(const RelativeValueChanger<float> * relative_resizer, float base, float factor) const;
	void initializeResizer(float value);

	// settings
	const SettingContainer & toolSettingContainer() const { return _tool_setting_container; }

	virtual void setGripPoint(const QPointF & p) = 0;
	virtual void setTipPoint(const QPointF & p) = 0;

	virtual QTransform * createScalingTransform();
	virtual QTransform * createSetupTransform();
	virtual QTransform * createParallelTransform();

	virtual float ratio() const = 0;

private:
	QList<SceneTouchPoint *> _additional_points;
	DefiningTouch * _defining_touch;
	//QPointF _grip_point;
	//QPointF _tip_point;
	const SettingContainer & _tool_setting_container;

};

#endif