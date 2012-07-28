#include "magnifyingglassoptimizer.h"

// --------------------------------------------------------
MagnifyingGlassOptimizer::MagnifyingGlassOptimizer(
	MagnifyingGlass * dynamic_glass,
	float scene_aspect_ratio,
	float desired_angle
)
{
	this->dynamic_glass = dynamic_glass;
	this->_scene_margin = (1.0f-(1.0f/scene_aspect_ratio))/2.0f;
	this->_desired_angle = desired_angle;
}

// --------------------------------------------------------
void MagnifyingGlassOptimizer::optimizeLastGlassPosition(float maximal_variation, QPointF * pole)
{
	static double prev_angle_increment = 0;
	//const float desired_angle = 0.0f;
	const double prev_radius = dynamic_glass->dstRadiusT();

	dynamic_glass->setAngle(_desired_angle);
	linearlyArrangeGlasses();

	// find biggest possible radius (satisfying both the inside and the desired condition) for angle = 0
	// postcondition of the following: radius is largest possible but still works
	adjustLastGlassToBiggestRadius(0.001f);

	//return;

	double radius = dynamic_glass->dstRadiusT();

	double angle_increment = 0.0f;
	double best_angle_increment = 0.0f;

	double radius_step;
	//const double compressibility = 0.0001;
	const double compressibility = 0.00033;

	Q_ASSERT(glassInsideBounds(dynamic_glass));
	double eps = 0.000001;

	//QMap<double, double> angles_by_reasonability;

	double min_unreasonable_angle = 0;
	double min_unreasonable_ratio = DBL_MAX;

	/*if(pole != NULL)
	{
		qDebug() << "has pole" << *pole;
	}
	else
	{
		qDebug() << "HAS NO POLE!";
	}*/
	while(abs(angle_increment) < maximal_variation)
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
		dynamic_glass->setAngle(angle);

		//if(abs(angle_increment - prev_angle_increment) + eps > abs(prev_angle_increment))
		if((angle_increment - prev_angle_increment) * sign > eps)
		{
			// this angle should at least support one more step than the previous one,
			// otherwise we prefer sticking with the last one.
			radius += radius_step;
			dynamic_glass->setDstRadius(radius);
			linearlyArrangeGlasses();
			if(!glassInsideBounds(dynamic_glass))
			{
				radius -= radius_step;
				dynamic_glass->setDstRadius(radius);
				linearlyArrangeGlasses();
				continue;
			}
		}
		else
		{
			// this angle should at least support the current radius
			dynamic_glass->setDstRadius(radius);
			linearlyArrangeGlasses();
			if(!glassInsideBounds(dynamic_glass))
			{
				//qDebug() << "ANGLE REJECTED:"<<angle_increment;
				continue;
			}
		}

		//qDebug() << "testing angle increment" << angle_increment;

		// at this point, the new angle is at least SLIGHTLY (by one radius step) better
		// than the old angle. but:
		//  (1) we dont know yet exactly how good it is.
		//  (2) it can still be unreasonable (large angle step for small radius gain).
		Q_ASSERT(glassInsideBounds(dynamic_glass));

		// -> (1) let's push the radius to the limit to check how good this angle really is.
		//  -> (1a) make as big as possible subject only to the screen border
		growGlassMaximally(dynamic_glass, radius_step);
		Q_ASSERT(glassAlmostInsideBounds(dynamic_glass));
		radius = dynamic_glass->dstRadiusT();


		// if we have a pole and it is outside of the glass, reject the angle 
		//if(pole != NULL)
		//{
		//	qDebug() << "has pole" << *pole;
		//}
		if(pole != NULL && !dynamic_glass->contains(*pole))
		{
			// TODO IN PAPER: support multiple poles
			//qDebug() << "continuing due to pole" << *pole;
			continue;
		}


		//  -> (1b) we don't need to go bigger than the desired radius
		if(radius > dynamic_glass->dstRadiusD())
		{
			//qDebug() << "breaking at radius" << radius << "desired" << last_mg->dstRadiusD();
			radius = dynamic_glass->dstRadiusD();
			dynamic_glass->setDstRadius(radius);
			linearlyArrangeGlasses();
			Q_ASSERT(glassInsideBounds(dynamic_glass));
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

	dynamic_glass->setAngle(_desired_angle + best_angle_increment);
	prev_angle_increment = best_angle_increment;
	dynamic_glass->setDstRadius(radius);
	linearlyArrangeGlasses();


	if(!glassInsideBounds(dynamic_glass))
	{
		shrinkGlassMinimally(dynamic_glass, 0.001f);
	}
	//growGlassMaximally(dynamic_glass, 0.001f); // added june 4

	Q_ASSERT(glassInsideBounds(dynamic_glass));
	//qDebug() << "final incr" << best_angle_increment;
	//qDebug() << "final angle: " << angle;
	//qDebug() << "final rad" << last_mg->dstRadiusT();
}

// --------------------------------------------------------
bool MagnifyingGlassOptimizer::glassInsideBounds(const MagnifyingGlass * mg) const
{
	float r = mg->dstRadiusT();
	QRectF rect(r, r+_scene_margin, 1-2*r, 1-2*r-2*_scene_margin);
	if(rect.width() <= 0 || rect.height() <= 0) return false;
	return rect.contains(mg->dstCenterT());
}

// --------------------------------------------------------
bool MagnifyingGlassOptimizer::glassAlmostInsideBounds(const MagnifyingGlass * mg) const
{
	float eps = 0.0001f;
	float r = mg->dstRadiusT();
	QRectF rect(r-eps, r+_scene_margin-eps, 1-2*r+2*eps, 1-2*r-2*_scene_margin+2*eps);
	if(rect.width() <= 0 || rect.height() <= 0) return false;
	return rect.contains(mg->dstCenterT());
}

// --------------------------------------------------------
void MagnifyingGlassOptimizer::linearlyArrangeGlasses()
{
	// arrange all magnifying glasses linearly with current angle
	//QPointF base_center = _desired_base_center != NULL ? *_desired_base_center : base_glass->srcCenterT();
	//QPointF current_pos = base_glass->dstCenterT();// - base_glass->dstRightVector();
	//QPointF current_pos = base_glass->srcCenterT() - base_glass->dstRightVector();

	//qDebug() << "startpos" << current_pos << base_glass->srcCenterT() << base_glass->dstRightVector();
	//foreach(MagnifyingGlass * mg, _magnifying_glasses)
	//{
	//	QPointF current_offset = mg->dstRightVector();
	//	current_pos += current_offset;
	//	mg->setDstCenter(current_pos);
	//	current_pos += current_offset;
	//}
	//qDebug() << "done rearranging";
	//dynamic_glass->setDstCenter(base_glass->dstCenterT() + base_glass->dstRightVector() + dynamic_glass->dstRightVector());
	QPointF off = QLineF(QPointF(), dynamic_glass->dstRightVector()).unitVector().pointAt(dynamic_glass->srcRadiusT() + dynamic_glass->dstRadiusT());

	//QPointF off = QLineF(QPointF(), dynamic_glass->dstRightVector()).unitVector().pointAt(0.025 + dynamic_glass->dstRadiusT());
	dynamic_glass->setDstCenter(dynamic_glass->srcCenterT() + off);
}

// --------------------------------------------------------
void MagnifyingGlassOptimizer::adjustLastGlassToBiggestRadius( float radius_step )
{
	//float desired_radius = last_mg->dstRadiusD();
	dynamic_glass->setAngle(_desired_angle);

	//float radius = last_mg->dstRadiusT();

	if(glassInsideBounds(dynamic_glass))
	{
		// we would like to grow
		growGlassMaximallyOrToDesired(dynamic_glass, radius_step);
	}
	else
	{
		// we would like to shrink
		shrinkGlassMinimally(dynamic_glass, radius_step);
	}
}


// --------------------------------------------------------
void MagnifyingGlassOptimizer::shrinkGlassMinimally( MagnifyingGlass * mg, float radius_step )
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
void MagnifyingGlassOptimizer::growGlassMaximallyOrToDesired( MagnifyingGlass * mg, float radius_step )
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
void MagnifyingGlassOptimizer::growGlassMaximally( MagnifyingGlass * mg, float radius_step )
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
