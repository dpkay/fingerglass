#include "magnifyingglassmanager.h"
#include "settings.h"
#include <QtDebug>
#include <cfloat>

// --------------------------------------------------------
MagnifyingGlassManager::MagnifyingGlassManager(
	const QPointF & init_center,
	float ratio,
	float scene_aspect_ratio,
	float desired_angle,
	MagnifyingGlass * zoom_glass
)
{
	// there is always one default glass with ratio 1:1
	MagnifyingGlass * base_glass;
	base_glass = new MagnifyingGlass(
		Settings::instance()->mwEllipseRatio(),
		zoom_glass == NULL ? 0.05f : zoom_glass->srcRadiusT(),
		1.0f,
		init_center);
	_magnifying_glasses.push_back(base_glass);

	// now, add the glasses which actually magnify (ratio > 1)
	if(zoom_glass == NULL)
	{
		for(int i=0; i<Settings::instance()->mwGlassCount(); ++i)
		{
			ratio *= Settings::instance()->mwMagnificationMultiplier();
			_magnifying_glasses.push_back
			(
				new MagnifyingGlass
				(
					Settings::instance()->mwEllipseRatio(),
					0.0f, // gets overwritten later in MagnifyingWidgetController::updateMagnifyingGlasses
					// if the dst radius is 0.0f in the beginning
					ratio,
					init_center
				)
			);
		}
	}
	else
	{
		_magnifying_glasses.push_back(zoom_glass);
	}
	this->_base_glass_visible = true;
	this->_desired_base_center = NULL;
	this->_scene_aspect_ratio = scene_aspect_ratio;
	this->_scene_margin = (1.0f-(1.0f/scene_aspect_ratio))/2.0f;
	this->_desired_angle = desired_angle;
	this->_maximal_variation = 180;
	this->_pole = 0;
}

// --------------------------------------------------------
const QVector<MagnifyingGlass *> & MagnifyingGlassManager::magnifyingGlasses()
{
	return _magnifying_glasses;
}

// --------------------------------------------------------
void MagnifyingGlassManager::setSrcPos( QPointF pos )
{
	foreach(MagnifyingGlass * mg, _magnifying_glasses)
	{
		mg->setSrcCenter(pos);
	}
}

// --------------------------------------------------------
void MagnifyingGlassManager::setAngle( float angle )
{
	foreach(MagnifyingGlass * mg, _magnifying_glasses)
	{
		mg->setAngle(angle);
	}
	linearlyArrangeGlasses();
}

// --------------------------------------------------------
const MagnifyingGlass * MagnifyingGlassManager::magnifyingGlassByScreenPos( const QPointF & pos )
{
	foreach(MagnifyingGlass * mg, _magnifying_glasses)
	{
		// transform into magnifying glass space
		QPointF p = mg->dstCenterT() - pos;
		p = mg->invRotationMatrix().map(p);
		p.setX(p.x()*mg->majorMinor());

		// perform inside/outside test
		if(QLineF(QPointF(), p).length() < mg->dstRadius())
		{
			return mg;
		}
	}

	// if nothing found
	return NULL;
}

// --------------------------------------------------------
void MagnifyingGlassManager::setSrcByPointPair( const QPointF & p1, const QPointF & p2 )
{
	// compute normalized difference vector
	QPointF diff = p2 - p1;
	float diff_len = sqrt(diff.x()*diff.x()+diff.y()*diff.y());
	diff /= diff_len;

	// compute angle
	if(!Settings::instance()->mwResizable())
	{
		float angle = atan2(-diff.x(), -diff.y()) * 180 / 3.1415f;
		setAngle(angle);
	}

	// compute radius and update position
	setSrcRadius(diff_len/2);
	setSrcPos((p1+p2)/2);

	//updateDstPositions();
}

//// --------------------------------------------------------
//void MagnifyingGlassManager::setSrcAndUpdateDst( const QPointF & pos, float radius, float angle )
//{
//	setAngle(angle);
//	setSrcRadius(radius);
//	setSrcPos(pos);
//	updateDstPositions();
//}

// --------------------------------------------------------
void MagnifyingGlassManager::setSrcRadius( float radius )
{
	foreach(MagnifyingGlass * mg, _magnifying_glasses)
	{
		bool maintain_ratio = Settings::instance()->mwSrcResizingMaintainsRatio();
		if(_base_glass_visible && mg == _magnifying_glasses.first())
		{
			maintain_ratio = true;
		}
		mg->setSrcRadius(radius, maintain_ratio);
	}
}

// --------------------------------------------------------
MagnifyingGlass * MagnifyingGlassManager::singleMagnifyingGlass()
{
	Q_ASSERT(Settings::instance()->mwGlassCount() == 1);
	return _magnifying_glasses.back();
}

// --------------------------------------------------------
bool MagnifyingGlassManager::glassInsideBounds(const MagnifyingGlass * mg, bool debug) const
{
	float r = mg->dstRadiusT();
	QRectF rect(r, r+_scene_margin, 1-2*r, 1-2*r-2*_scene_margin);
	if(rect.width() <= 0 || rect.height() <= 0) return false;
	if(debug)
		qDebug() << QRectF(r, r+_scene_margin, 1-2*r, 1-2*r-2*_scene_margin);
	return rect.contains(mg->dstCenterT());
}

// --------------------------------------------------------
bool MagnifyingGlassManager::glassAlmostInsideBounds(const MagnifyingGlass * mg, bool debug) const
{
	float eps = 0.0001f;
	float r = mg->dstRadiusT();
	QRectF rect(r-eps, r+_scene_margin-eps, 1-2*r+2*eps, 1-2*r-2*_scene_margin+2*eps);
	if(rect.width() <= 0 || rect.height() <= 0) return false;
	if(debug)
		qDebug() << QRectF(r-eps, r+_scene_margin-eps, 1-2*r+2*eps, 1-2*r-2*_scene_margin+2*eps);
	return rect.contains(mg->dstCenterT());
}

// --------------------------------------------------------
 void MagnifyingGlassManager::updateDstPositions(bool update_last)
{
	// first attempt without any correction. then check whether this
	// makes the last glass lie out of the screen. if not, we are fine.
	linearlyArrangeGlasses(update_last);
	if(_base_glass_visible)
	{
		optimizeBaseGlassPosition();
	}
	if(update_last)
	{
		//optimizeLastGlassPosition();
		MagnifyingGlassOptimizer optimizer(_magnifying_glasses.back(), _scene_aspect_ratio, _desired_angle);
		optimizer.optimizeLastGlassPosition(_maximal_variation, _pole);
	}
}

// --------------------------------------------------------
void MagnifyingGlassManager::linearlyArrangeGlasses(bool update_last)
{
	// we start at the outer border of the first magnifying glass
	const MagnifyingGlass * base_glass = _magnifying_glasses.value(0);

	// arrange all magnifying glasses linearly with current angle
	QPointF base_center = _desired_base_center != NULL ? *_desired_base_center : base_glass->srcCenterT();
	QPointF current_pos = base_center - base_glass->dstRightVector();
	//QPointF current_pos = base_glass->srcCenterT() - base_glass->dstRightVector();

	//qDebug() << "startpos" << current_pos << base_glass->srcCenterT() << base_glass->dstRightVector();
	foreach(MagnifyingGlass * mg, _magnifying_glasses)
	{
		QPointF current_offset = mg->dstRightVector();
		current_pos += current_offset;
		mg->setDstCenter(current_pos);
		current_pos += current_offset;

		if(!update_last) break;
	}
	//qDebug() << "done rearranging";
}


// --------------------------------------------------------
void MagnifyingGlassManager::adjustLastGlassToBiggestRadius( float radius_step )
{
	MagnifyingGlass * last_mg = _magnifying_glasses.back();
	//float desired_radius = last_mg->dstRadiusD();
	setAngle(_desired_angle);

	//float radius = last_mg->dstRadiusT();

	if(glassInsideBounds(last_mg))
	{
		// we would like to grow
		growGlassMaximallyOrToDesired(last_mg, radius_step);
	}
	else
	{
		// we would like to shrink
		shrinkGlassMinimally(last_mg, radius_step);
	}
}


// --------------------------------------------------------
void MagnifyingGlassManager::shrinkGlassMinimally( MagnifyingGlass * mg, float radius_step )
{
	float radius = mg->dstRadiusT();
	forever
	{
		radius -= radius_step;
		mg->setDstRadius(radius);
		linearlyArrangeGlasses();
		if(glassInsideBounds(mg))
		{
			break;
		}
	}
}

// --------------------------------------------------------
void MagnifyingGlassManager::growGlassMaximallyOrToDesired( MagnifyingGlass * mg, float radius_step )
{
	float radius = mg->dstRadiusT();
	forever
	{
		Q_ASSERT(glassInsideBounds(mg));
		float last_safe_radius = radius;
		radius += radius_step;
		mg->setDstRadius(radius);
		linearlyArrangeGlasses();
		if(!glassInsideBounds(mg) || radius > mg->dstRadiusD())
		{
			//radius -= radius_step;
			radius = last_safe_radius;
			mg->setDstRadius(radius);
			linearlyArrangeGlasses();
			Q_ASSERT(glassInsideBounds(mg));
			break;
		}
	}
}


// --------------------------------------------------------
void MagnifyingGlassManager::growGlassMaximally( MagnifyingGlass * mg, float radius_step )
{
	float radius = mg->dstRadiusT();
	forever
	{
		Q_ASSERT(glassInsideBounds(mg));
		float last_safe_radius = radius;
		radius += radius_step;
		mg->setDstRadius(radius);
		linearlyArrangeGlasses();
		if(!glassInsideBounds(mg))
		{
			//radius -= radius_step;
			radius = last_safe_radius;
			mg->setDstRadius(radius);
			linearlyArrangeGlasses();
			Q_ASSERT(glassAlmostInsideBounds(mg));
			break;
		}
	}
}

// --------------------------------------------------------
void MagnifyingGlassManager::optimizeLastGlassPosition()
{
	MagnifyingGlass * last_mg = _magnifying_glasses.back();

	static double prev_angle_increment = 0;
	//const float desired_angle = 0.0f;
	const double prev_radius = last_mg->dstRadiusT();

	// find biggest possible radius (satisfiyng both the inside and the desired condition) for angle = 0
	// postcondition of the following: radius is largest possible but still works
	adjustLastGlassToBiggestRadius(0.001f);

	double radius = last_mg->dstRadiusT();

	double angle_increment = 0.0f;
	double best_angle_increment = 0.0f;

	double radius_step;
	const double compressibility = 0.0001;

	Q_ASSERT(glassInsideBounds(last_mg));
	double eps = 0.000001;

	//QMap<double, double> angles_by_reasonability;

	double min_unreasonable_angle = 0;
	double min_unreasonable_ratio = DBL_MAX;

	while(abs(angle_increment) < 180)
	{
		double sign = angle_increment > 0 ? 1 : -1;
		double abs_angle_dist = abs(abs(angle_increment) - abs(prev_angle_increment));
		abs_angle_dist = qMin(qMax(abs_angle_dist, 0.05), 10.0);

		// compute next angle
		double angle_increment_increment;
		angle_increment_increment = abs_angle_dist/100; // will be between 0.0005 and 0.1
		angle_increment = (abs(angle_increment) + angle_increment_increment) * (-sign);
		sign *= -1;

		// compute radius step size
		radius_step = sqrt(abs_angle_dist)/1000; // will be between 0.0002 and 0.0032

		bool stop_search = false;

		float angle = _desired_angle + angle_increment;
		setAngle(angle);

		//if(abs(angle_increment - prev_angle_increment) + eps > abs(prev_angle_increment))
		if((angle_increment - prev_angle_increment) * sign > eps)
		{
			// this angle should at least support one more step than the previous one,
			// otherwise we prefer sticking with the last one.
			radius += radius_step;
			last_mg->setDstRadius(radius);
			linearlyArrangeGlasses();
			if(!glassInsideBounds(last_mg))
			{
				radius -= radius_step;
				last_mg->setDstRadius(radius);
				linearlyArrangeGlasses();
				continue;
			}
		}
		else
		{
			// this angle should at least support the current radius
			last_mg->setDstRadius(radius);
			linearlyArrangeGlasses();
			if(!glassInsideBounds(last_mg))
			{
				//qDebug() << "ANGLE REJECTED:"<<angle_increment;
				continue;
			}
		}

		// if we have a pole and it is outside of the glass, reject the angle 
		if(_pole != NULL && !last_mg->contains(*_pole))
		{
			// TODO IN PAPER: support multiple poles
			continue;
		}

		//qDebug() << "testing angle increment" << angle_increment;

		// at this point, the new angle is at least SLIGHTLY (by one radius step) better
		// than the old angle. but:
		//  (1) we dont know yet exactly how good it is.
		//  (2) it can still be unreasonable (large angle step for small radius gain).
		Q_ASSERT(glassInsideBounds(last_mg));

		// -> (1) let's push the radius to the limit to check how good this angle really is.
		//  -> (1a) make as big as possible subject only to the screen border
		growGlassMaximally(last_mg, radius_step);
		Q_ASSERT(glassAlmostInsideBounds(last_mg));
		radius = last_mg->dstRadiusT();

		//  -> (1b) we don't need to go bigger than the desired radius
		if(radius > last_mg->dstRadiusD())
		{
			//qDebug() << "breaking at radius" << radius << "desired" << last_mg->dstRadiusD();
			radius = last_mg->dstRadiusD();
			last_mg->setDstRadius(radius);
			linearlyArrangeGlasses();
			Q_ASSERT(glassInsideBounds(last_mg));
			stop_search = true;
		}

		// -> (2) let's see if this is reasonable
		double delta_radius_angle_ratio = (radius-prev_radius)/abs(angle_increment-prev_angle_increment);
		double d_angle_abs = (angle_increment-prev_angle_increment) * sign;
		if(d_angle_abs < eps)
		{
			// we always welcome decreasing angles!
			best_angle_increment = angle_increment;
		}
		else
		if(delta_radius_angle_ratio > compressibility)
		{
			// we allow compression to a certain degree
			best_angle_increment = angle_increment;

			//qDebug() << "storing with reasonability" << reasonability << "radius" << radius << "prevradius" << prev_radius << "a_incr" << angle_increment << "prev_a_incr" << prev_angle_increment;
		}
		else
		{
			// everything else we just accept as a last resort
			if(delta_radius_angle_ratio < min_unreasonable_ratio)
			{
				min_unreasonable_ratio = delta_radius_angle_ratio;
				min_unreasonable_angle = angle_increment;
			}
		}

		if(stop_search)
		{
			break;
		}

	}

	if(best_angle_increment == 0)
	{
		best_angle_increment = min_unreasonable_angle;
	}

	setAngle(_desired_angle + best_angle_increment);
	prev_angle_increment = best_angle_increment;
	last_mg->setDstRadius(radius);
	linearlyArrangeGlasses();


	if(!glassInsideBounds(last_mg))
	{
		shrinkGlassMinimally(last_mg, 0.001f);
	}

	qDebug() << "desired" << _desired_angle << "last" << last_mg->angle() << "right" << last_mg->dstRightVector();


	Q_ASSERT(glassInsideBounds(last_mg));
	//qDebug() << "final incr" << best_angle_increment;
	//qDebug() << "final angle: " << angle;
	//qDebug() << "final rad" << last_mg->dstRadiusT();
}

// --------------------------------------------------------
void MagnifyingGlassManager::optimizeBaseGlassPosition()
{
	MagnifyingGlass * base_glass = _magnifying_glasses.value(0);

	if(base_glass->dstRadiusT() > 0.5f-_scene_margin)
	{
		setSrcRadius(0.5f-_scene_margin);
		base_glass->setDstRadius(0.5f-_scene_margin);
	}


	const QPointF & old_center = base_glass->dstCenterT();
	QPointF center = old_center;
	const float radius = base_glass->dstRadiusT();

	if(old_center.x() < radius) center.setX(radius);
	if(old_center.x() > 1-radius) center.setX(1-radius);
	if(old_center.y() < _scene_margin+radius) center.setY(_scene_margin+radius);
	if(old_center.y() > 1-_scene_margin-radius) center.setY(1-_scene_margin-radius);

	setSrcPos(center);

}

// --------------------------------------------------------
MagnifyingGlass * MagnifyingGlassManager::baseGlass()
{
	return _magnifying_glasses.value(0);	
}

// --------------------------------------------------------
const MagnifyingGlass * MagnifyingGlassManager::baseGlass() const
{
	return _magnifying_glasses.value(0);	
}

// --------------------------------------------------------
void MagnifyingGlassManager::setBaseGlassVisible( bool maintains_ratio )
{
	this->_base_glass_visible = maintains_ratio;
}

// --------------------------------------------------------
MagnifyingGlassManager::~MagnifyingGlassManager()
{
//	qDebug() << "mgm dest" << this;
	foreach(MagnifyingGlass * mg, _magnifying_glasses)
	{
		delete mg;
	}
	delete _desired_base_center;
}

// --------------------------------------------------------
void MagnifyingGlassManager::setDesiredBaseGlassCenter( const QPointF & p )
{
	if(_desired_base_center == NULL)
	{
		_desired_base_center = new QPointF;
	}
	*_desired_base_center = p;
}

// --------------------------------------------------------
void MagnifyingGlassManager::setDesiredAngle( float angle )
{
	_desired_angle = angle;
}

// --------------------------------------------------------
void MagnifyingGlassManager::setDesiredMaximalVariation( float maximal_variation )
{
	_maximal_variation = maximal_variation;
}

// --------------------------------------------------------
bool MagnifyingGlassManager::singleMagnifyingGlassHasMaxSize() const
{
	float eps = 0.01f;
	float r = _magnifying_glasses.back()->dstRadiusT();
	QRectF rect(r, r+_scene_margin, 1-2*r, 1-2*r-2*_scene_margin);
	qDebug() << rect.height();
	return (rect.width() <= eps || rect.height() <= eps);
}

// --------------------------------------------------------
void MagnifyingGlassManager::goBackIntoBounds()
{
	MagnifyingGlass * mg = _magnifying_glasses.back();
	if(mg->dstRadiusT() > 0.5f-_scene_margin)
	{
		mg->setDstRadius(0.5f-_scene_margin);
	}

	if(mg->dstCenterT().x() - mg->dstRadiusT() < 0.0f)
	{
		mg->setDstCenter(QPointF(mg->dstRadiusT(), mg->dstCenterT().y()));
	}
	if(mg->dstCenterT().x() + mg->dstRadiusT() > 1.0f)
	{
		mg->setDstCenter(QPointF(1.0f - mg->dstRadiusT(), mg->dstCenterT().y()));
	}
	if(mg->dstCenterT().y() - mg->dstRadiusT() < _scene_margin)
	{
		mg->setDstCenter(QPointF(mg->dstCenterT().x(), _scene_margin + mg->dstRadiusT()));
	}
	if(mg->dstCenterT().y() + mg->dstRadiusT() > 1.0f - _scene_margin)
	{
		mg->setDstCenter(QPointF(mg->dstCenterT().x(), 1.0f - _scene_margin - mg->dstRadiusT()));
	}
}

// --------------------------------------------------------
void MagnifyingGlassManager::setPole( const QPointF & p )
{
	if(_pole != NULL)
	{
		delete _pole;
	}
	_pole = new QPointF(p);
	//qDebug() << "setting pole" << p;
}

// --------------------------------------------------------
void MagnifyingGlassManager::removePole()
{
	if(_pole != NULL)
	{
		delete _pole;
		_pole = NULL;
	}
}