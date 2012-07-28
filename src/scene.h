#ifndef scene_h__
#define scene_h__

#include "settingcontainer.h"
#include "touchwidgetmanager.h"
#include "rigidwidgetcontroller.h"
#include "touchwidget.h"

class Scene
{
public:
	Scene(SettingContainer * setting_container);
	void setup();
	RigidWidgetController * randomBall() const;
	const SettingContainer * settingContainer() const { return _setting_container; }
	void setTouchWidgetManager(TouchWidgetManager * twm);
	RigidWidgetController * createTarget(const QPointF & center);
	void remove(RigidWidgetController * rwc);
	const TouchWidgetManager * touchWidgetManager() const { return _twm; }
	void createBallTargetPair(RigidWidgetController ** ball_rwc, RigidWidgetController ** target_rwc, int exponent);

protected:
	void createBackground();
	void createBalls();
	const QList<RigidWidgetController *> & balls() const;
	static QPointF randomRectPos(const QRectF & range);
	QPointF randomRingPos(const QPointF & center, float inner_radius, float outer_radius);
	bool isOverlappingWithAnyBall(const QPointF & pos) const;

private:
	SettingContainer * _setting_container;
	QList<RigidWidgetController *> _balls;
	TouchWidgetManager * _twm;
	QRectF _scene_rect;

	friend QDebug operator<<(QDebug dbg, const Scene &s);
};


#endif // scene_h__