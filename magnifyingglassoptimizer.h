#ifndef magnifyingglassoptimizer_h__
#define magnifyingglassoptimizer_h__

#include "magnifyingglass.h"

class MagnifyingGlassOptimizer
{
public:
	MagnifyingGlassOptimizer(
		MagnifyingGlass * dynamic_glass,
		float scene_aspect_ratio,
		float desired_angle
	);

	void optimizeLastGlassPosition(float maximal_variation = 180, QPointF * pole = NULL);

protected:
	// helper function to detect out-of-bounds situations
	bool glassInsideBounds(const MagnifyingGlass * mg) const;
	bool glassAlmostInsideBounds(const MagnifyingGlass * mg) const;
	
	// helper functions to move glasses rigidly
	void linearlyArrangeGlasses();

	// helper functions to change size
	void adjustLastGlassToBiggestRadius(float radius_step);
	void shrinkGlassMinimally( MagnifyingGlass * mg, float radius_step );
	void growGlassMaximallyOrToDesired( MagnifyingGlass * mg, float radius_step );
	void growGlassMaximally( MagnifyingGlass * mg, float radius_step );

private:
	MagnifyingGlass * dynamic_glass;
	float _scene_margin;
	float _desired_angle;

};

#endif // magnifyingglassoptimizer_h__