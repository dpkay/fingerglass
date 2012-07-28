#ifndef magnifyingglassmanager_h__
#define magnifyingglassmanager_h__

#include "magnifyingglass.h"
#include "magnifyingglassoptimizer.h"
#include <QMatrix>

class MagnifyingGlassManager
{
public:
	MagnifyingGlassManager(const QPointF & init_center, float ratio, float scene_aspect_ratio, float desired_angle, MagnifyingGlass * zoom_glass = NULL);
	~MagnifyingGlassManager();
	const QVector<MagnifyingGlass *> & magnifyingGlasses();
	void setSrcPos(QPointF pos);
	void setSrcRadius(float radius);
	//void setDstRadius(float radius);
	void setAngle(float angle);
	void setSrcByPointPair(const QPointF & p1, const QPointF & p2);
	//void setSrcByPointPair(const QPointF & pos, float radius, float angle);
	void updateDstPositions(bool update_base = true);

	// magnifying glass requests
	MagnifyingGlass * singleMagnifyingGlass();
	const MagnifyingGlass * magnifyingGlassByScreenPos(const QPointF & pos);
	MagnifyingGlass * baseGlass();
	const MagnifyingGlass * baseGlass() const;

	void setDesiredAngle(float angle);
	void setDesiredMaximalVariation(float maximal_variation);

	float desiredAngle() const { return _desired_angle; }

	void setBaseGlassVisible(bool visible);

	void setDesiredBaseGlassCenter(const QPointF & p);
	bool singleMagnifyingGlassHasMaxSize() const;
	void goBackIntoBounds();
	float sceneMargin() const { return _scene_margin; }
	void setPole(const QPointF & p);
	void removePole();

protected:
	bool glassInsideBounds(const MagnifyingGlass * mg, bool debug=false) const;
	bool glassAlmostInsideBounds(const MagnifyingGlass * mg, bool debug=false) const;
	void linearlyArrangeGlasses(bool update_last = true);
	
	void adjustLastGlassToBiggestRadius(float radius_step);

	void shrinkGlassMinimally( MagnifyingGlass * mg, float radius_step );
	void growGlassMaximallyOrToDesired( MagnifyingGlass * mg, float radius_step );
	void growGlassMaximally( MagnifyingGlass * mg, float radius_step );
	// positional optimization
	//enum OptimizationType
	//{
	//	FindFirstSolutionOptimization,
	//	FindBestApproximationOptimization
	//};
	//bool tryAngularOptimization(OptimizationType type);
	//bool trySizeOptimization();

	void optimizeBaseGlassPosition();
	void optimizeLastGlassPosition();


private:
	QVector<MagnifyingGlass *> _magnifying_glasses;
	bool _base_glass_visible;
	QPointF * _desired_base_center; // in cases where the base glass should not be the same as its src center
	float _scene_margin;
	float _desired_angle;
	float _scene_aspect_ratio; 
	float _maximal_variation;
	QPointF * _pole;

	friend class MagnifyingWidgetRenderer;
};

#endif // magnifyingglassmanager_h__