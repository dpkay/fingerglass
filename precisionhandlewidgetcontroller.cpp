#include "precisionhandlewidgetcontroller.h"
#include "rigidwidgetcontroller.h"
#include "variousattachments.h "
#include "globallog.h"

// --------------------------------------------------------
PrecisionHandleWidgetController::PrecisionHandleWidgetController(
	const SettingContainer & tool_setting_container
	) :
SingleHandedPrecisionWidgetController(tool_setting_container)
{
	_relative_resizer = NULL;
	_mg = NULL; 
	_transform = NULL;
	_initial_phase = true;
}

// --------------------------------------------------------
void PrecisionHandleWidgetController::updateAfterMovement()
{
	float length = QLineF(gripPoint(), tipPoint()).length();
	float ratio = _mg->ratio();
	_mg->setDstRadius(desiredMagnifyingGlassRadius());
	_mg->setSrcRadius(length/(ratio+1), false);
}

// --------------------------------------------------------
void PrecisionHandleWidgetController::resizeGlass()
{
	//float desired_ratio = desiredRelativeChangingValue(_relative_resizer, 2.0f, toolSettingContainer().floatProperty("resize_speed"));
	float desired_ratio = desiredRelativeChangingValue(_relative_resizer, 2.0f, 50.0f);
	desired_ratio = qMax(0.25f, desired_ratio);
	desired_ratio = qMin(8.0f, desired_ratio);
	_mg->setSrcRadiusByRatio(desired_ratio);
}

// --------------------------------------------------------
float PrecisionHandleWidgetController::desiredMagnifyingGlassRadius() const
{
	float length = QLineF(gripPoint(), tipPoint()).length();
	return _mg->ratio()/(_mg->ratio()+1)*length;
}

// --------------------------------------------------------
void PrecisionHandleWidgetController::updateAndTransformPoints()
{
	Q_ASSERT(definingTouch() != NULL);

	// for convenience
	SceneTouchPoint * p = definingTouch()->stp;

	// update grip/tip point and touchpoint
	setGripPoint(p->pos());
	if(definingTouch()->transformed_start_pos == NULL)
	{
		// in the very beginning
		p->transform(_transform->map(p->pos()), _transform->map(p->startPos()), p->rect().size()/_mg->ratio());
	}
	else
	{
		// after we know the definite tip position (adjusting to object location)
		p->transform(_transform->map(p->pos()), *definingTouch()->transformed_start_pos, p->rect().size()/_mg->ratio());
	}
	setTipPoint(p->pos());
	p->setVisibleInScene(dynamic_cast<RigidWidgetController *>(p->bottomWidgetConnection()->widgetController())==NULL);
}

// --------------------------------------------------------
PrecisionHandleWidgetController::~PrecisionHandleWidgetController()
{
	delete _mg;
	delete _transform;
	if(_relative_resizer != NULL)
	{
		delete _relative_resizer;
	}
}

// --------------------------------------------------------
bool PrecisionHandleWidgetController::acceptTouchPoint( const SceneTouchPoint & p ) const
{
	if(definingTouch() == NULL)
	{
		Q_ASSERT(_mg != NULL);
		return QLineF(p.pos(), _mg->dstCenterT()).length() < _mg->dstRadius();
	}
	else
	{
		return additionalPoints().size() < 2;
	}
}

// --------------------------------------------------------
void PrecisionHandleWidgetController::updateToolMode()
{
	if(_relative_resizer != NULL)
	{
		delete _relative_resizer;
		_relative_resizer = NULL;
	}
	if(_transform != NULL)
	{
		delete _transform;
	}
	_transform = new QTransform();

	if(additionalPoints().size() >= 1)
	{
		_initial_phase = false;
	}

	logger() << "shpw_updatemode" << this << additionalPoints().size();

	if(additionalPoints().size() == 0 && _mg->ratio() != 1.0f && !_initial_phase)
	{
		//_tool_mode = OneFingerMode;
		QPointF new_center = tipPoint() + (tipPoint() - gripPoint())/(_mg->ratio() - 1);
		_transform->translate(new_center.x(), new_center.y());
		_transform->scale(1.0f/_mg->ratio(), 1.0f/_mg->ratio());
		_transform->translate(-new_center.x(), -new_center.y());
		logger() << "shpw_onefingermode" << this << new_center << _mg->ratio();
	}
	else if(additionalPoints().size() == 2 || (additionalPoints().size() == 0 && _mg->ratio() == 1.0f))
	{
		//_tool_mode = ThreeFingerMode;
		QPointF offset = tipPoint() - gripPoint();
		_transform->translate(offset.x(), offset.y());
	}
	else if(additionalPoints().size() == 1 || _initial_phase)
	{
		//_tool_mode = TwoFingerSetupMode;
		_transform->translate(tipPoint().x(), tipPoint().y());
		_transform->scale(0.0f, 0.0f);
		_relative_resizer = new RelativeValueChanger<float>(_mg->ratio(), definingTouch());
		logger() << "shpw_setupmode" << this << _mg->ratio();
	}
	else
	{
		Q_ASSERT(false);
	}
}

// --------------------------------------------------------
void PrecisionHandleWidgetController::test(SceneTouchPoint * p)
{
	_mg->setSpeed(1.0f);
	setTipPoint(_mg->transform(p->untransformedPos()));
	setGripPoint(p->untransformedPos());

	QPointF new_center = tipPoint() + (tipPoint() - gripPoint())/(_mg->ratio() - 1);
	if(_transform != NULL)
	{
		delete _transform;
	}
	_transform = new QTransform();
	_transform->translate(new_center.x(), new_center.y());
	_transform->scale(1.0f/_mg->ratio(), 1.0f/_mg->ratio());
	_transform->translate(-new_center.x(), -new_center.y());
	_initial_phase = false;
}

// --------------------------------------------------------
void PrecisionHandleWidgetController::initializeMagnifyingGlass(SceneTouchPoint * stp)
{
	Q_ASSERT(_mg == NULL);
	_mg = new MagnifyingGlass(1.0f, 0.1f, toolSettingContainer().floatProperty("ratio"), stp->pos());
	_mg->setSpeed(FLT_MAX);
}

// --------------------------------------------------------
const SceneTouchPoint * PrecisionHandleWidgetController::resizingTouch() const
{
	if(additionalPoints().size() == 1)
	{
		return additionalPoints().front();
	}
	else
	{
		return NULL;
	}
}

// --------------------------------------------------------
void PrecisionHandleWidgetController::startClosingAnimation(RigidWidgetController * rwc)
{
	MagnifyingGlass * mg = new MagnifyingGlass(*_mg);
	mg->setSpeed(0.3f);
	mg->setDstCenter(rwc->pos());
	mg->setDstRadius(rwc->coreRadius());
	rwc->attach(new InteractionFinishedAttachment(rwc, mg));
}