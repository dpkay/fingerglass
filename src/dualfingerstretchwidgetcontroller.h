#ifndef dualfingerstretchwidgetcontroller_h__
#define dualfingerstretchwidgetcontroller_h__

#include "touchwidgetcontroller.h"
#include "magnifyingglass.h"
#include "settingcontainer.h"

class DualFingerStretchWidgetController : public TouchWidgetController
{
public:
	DualFingerStretchWidgetController(const SettingContainer & tool_setting_container);
	~DualFingerStretchWidgetController();

	// from TouchWidgetController
	virtual void touchPointPressed(SceneTouchPoint * p);
	virtual void touchPointMoved(SceneTouchPoint * p);
	virtual void touchPointReleased(SceneTouchPoint * p);
	virtual bool acceptTouchPoint(const SceneTouchPoint & p) const;
	virtual bool forwardTouchPoint(const SceneTouchPoint & p) const;

	// from Timed
	//virtual void timeStep();
	//virtual bool finished() const;

	// magnifying glass management
	void setDefiningTouch(SceneTouchPoint * stp);
	const MagnifyingGlass * magnifyingGlass() const { return _mg; }
	const SceneTouchPoint * resizingPoint() const { return _resizing_point; }

//protected:
//	void transformDefiningPoint();
protected:
	RigidWidgetController * attachedRigidWidgetController();

private:
	const SettingContainer & _tool_setting_container;
	SceneTouchPoint * _resizing_point;
	MagnifyingGlass * _mg;
	QTime * _release_time;
	QPointF * _attached_rwc_start_pos;

	struct DeprecatedDefiningPoint
	{
		// we need this struct because we need to have an updatable start position
		explicit DeprecatedDefiningPoint(SceneTouchPoint * stp_)
		{
			Q_ASSERT(stp!=NULL);
			stp=stp_;
			last_pos_untransformed=stp->pos();
			last_pos_transformed=stp->pos();
			last_start_pos_untransformed=stp->startPos();
			last_start_pos_transformed=stp->startPos();

			override_new_start_pos = NULL;
		}
		explicit DeprecatedDefiningPoint(SceneTouchPoint * stp_, const MagnifyingGlass * mg)
		{
			Q_ASSERT(stp!=NULL);
			stp=stp_;
			last_pos_untransformed=stp->pos();
			last_pos_transformed=mg->transform(stp->pos());
			last_start_pos_untransformed=stp->startPos();
			last_start_pos_transformed=mg->transform(stp->startPos());

			QSizeF new_size = stp->rect().size()/mg->ratio();
			stp->transform(last_pos_transformed, last_start_pos_transformed, new_size);

			override_new_start_pos = NULL;
		}
		SceneTouchPoint * stp;
		void transform(float ratio)
		{
			// compute new pos
			QPointF delta_pos = stp->pos() - last_pos_untransformed;
			QPointF new_pos = last_pos_transformed + delta_pos / ratio;

			// compute new start pos
			QPointF new_start_pos;
			if(override_new_start_pos != NULL)
			{
				new_start_pos = *override_new_start_pos;
				delete override_new_start_pos;
				override_new_start_pos = NULL;
				//qDebug() << "overriding to" << new_start_pos;
			}
			else
			{
				QPointF delta_start_pos = stp->startPos() - last_start_pos_untransformed;
				new_start_pos = last_start_pos_transformed + delta_start_pos / ratio;
				//qDebug() << "last and del and new" << last_start_pos_transformed << delta_start_pos << new_start_pos;
			}

			// compute new size
			QSizeF new_size = stp->rect().size()/ratio;

			// update
			last_pos_untransformed = stp->pos();
			last_start_pos_untransformed = stp->startPos();
			stp->transform(new_pos, new_start_pos, new_size);
			last_pos_transformed = stp->pos();
			last_start_pos_transformed = stp->startPos();
		}

		void transform(const QTransform & transform_)
		{
			//QTransform itransform_ = transform_.inverted()

			// compute
			//QPointF delta_pos = stp->pos() - last_start_pos_transformed
			QPointF new_pos = transform_.map(stp->pos());

			// compute new start pos
			QPointF new_start_pos = transform_.map(stp->startPos());

			// compute new size
			QSizeF new_size = transform_.mapRect(stp->rect()).size();

			// update
			last_pos_untransformed = stp->pos();
			last_start_pos_untransformed = stp->startPos();
			stp->transform(new_pos, new_start_pos, new_size);
			last_pos_transformed = stp->pos();
			last_start_pos_transformed = stp->startPos();
		}

		void reset()
		{
			last_pos_untransformed = last_pos_transformed;
			last_start_pos_untransformed = last_start_pos_transformed;
		}

		void setStartPos(const QPointF & pos)
		{
			override_new_start_pos = new QPointF(pos);
			//qDebug() << "setStartPos" << pos << stp->startPos();
			//last_start_pos_transformed = pos;// - (stp->pos()-last_pos_untransformed
			////last_start_pos_untransformed = QPointF();
			//last_start_pos_untransformed = stp->startPos();
		}

	private:
		QPointF last_pos_untransformed;
		QPointF last_pos_transformed;
		QPointF last_start_pos_untransformed;
		QPointF last_start_pos_transformed;

		QPointF * override_new_start_pos;

	};
	DeprecatedDefiningPoint * _defining_point;

};

#endif // dualfingerstretchwidgetcontroller_h__