#ifndef rigidwidgetcontroller_h__
#define rigidwidgetcontroller_h__

class RigidWidgetController;

enum SelectionMode
{
	DirectPointSelection,
	DirectAreaSelection,
	IndirectPointSelection,
	IndirectBubbleSelection
};

#include "touchwidgetcontroller.h"
#include "afterglowbubble.h"
#include "timed.h"

class RigidWidgetController : public TouchWidgetController
{
public:
	// TODO: support constraints for sliders
	RigidWidgetController(TouchWidgetType type);
	~RigidWidgetController();

	// touch event handling
	void touchPointReleased( SceneTouchPoint * p );
	void touchPointMoved( SceneTouchPoint * p );
	void touchPointPressed( SceneTouchPoint * p );
	bool acceptTouchPoint(const SceneTouchPoint & p) const;
	bool forwardTouchPoint(const SceneTouchPoint & p) const;

	// drawing related stuff
	const QPointF & pos() const { return _pos; }
	void setPos(const QPointF & pos) { this->_pos = pos;}
	float totalRadius() const { return coreRadius() * (isHaloVisible() ? 5.0f : 1.0f); }
	float coreRadius() const { return this->_radius; }
	QSizeF totalSize() const { return 2*QSizeF(totalRadius(), totalRadius()); }
	QSizeF coreSize() const { return 2*QSizeF(coreRadius(), coreRadius()); }
	void setRadius(float radius) { this->_radius = radius; }
	bool isMovable() const { return _movable; }
	void setMovable(bool movable) { this->_movable = movable; }
	bool isMoving() const;
	//bool needsHalo(const TouchPoint & p) const;
	void setTouchPointHaloVisible(bool visible, const SceneTouchPoint * stp);


	bool isHaloVisible() const;
	const AfterGlowBubble * afterGlowBubble() const;

	void setAttachedToHandleWidget(bool attached);
	bool isAttachedToHandleWidget() const { return _attached_to_handle_widget; }

	void timeStep();
	bool finished() const;

	void setHighlighted(bool highlighted) { this->_highlighted = highlighted; }
	bool isHighlighted() const { return this->_highlighted; }

	// bubble related stuff
	float intersectingDistance(const QPointF & p);
	float containmentDistance(const QPointF & p);

	//void setSelectableByBubbleOnly(bool val) { _selectable_by_bubble_only = val; }
	void setSelectionMode(SelectionMode mode) { _selection_mode = mode; }

private:
	QPointF _pos;
	float _radius;
	QPointF * _original_pos;
	bool _movable;
	bool _touch_point_halo_visible;
	bool _attached_to_handle_widget;
	bool _highlighted;
	//bool _selectable_by_bubble_only;
	SelectionMode _selection_mode;
	//bool _after_glow_halo_visible;
	AfterGlowBubble * _after_glow_bubble;

};

#endif // rigidwidget_h__