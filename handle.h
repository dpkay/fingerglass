#ifndef handle_h__
#define handle_h__

#include <QSizeF>
#include <QPointF>
#include <QtGlobal>
#include "warping.h"
#include "magnifyingglass.h"

class Handle
{
public:
	Handle();
	//virtual ~Handle() = 0;
	virtual void setPos(const QPointF & pos) = 0;
	virtual const QPointF & posT() const = 0;
	virtual float cdRatio() const = 0;
	virtual bool isAbsolute() const = 0;

	// original pos (used for dragging)
	const QPointF & originalPos() const;
	void createOriginalPos();
	void resetOriginalPos();
	void setOriginalPos(const QPointF & pos);
	void removeOriginalPos();

private:
	//QPointF _pos;
	QPointF * _original_pos;
	//float _cd_ratio;
	//bool _is_absolute;
};

class AbsoluteHandle : public Handle
{
public:
	AbsoluteHandle(const QPointF & pos, const float radius);
	~AbsoluteHandle() {}
	float cdRatio() const { return 1.0f; }
	bool isAbsolute() const { return true; }
	void setPos(const QPointF & pos) { _pos_w.setTarget(pos); /*qDebug() << "absolutehandle moved to" << pos;*/}
	const QPointF & posT() const { return _pos_w.targetValue(); }
	const QPointF & pos() const { return _pos_w.value(); }

	//const QSizeF & size() const { return _size; }
	//const float radius() const { Q_ASSERT(fabs(_size.height()-_size.width())<1e-5); return _size.width()/2; }
	//void setRadius(float radius) { _size = QSizeF(2*radius, 2*radius); }
	const QSizeF size() const { return QSizeF(2*radius(), 2*radius()); }
	const float radius() const { return _radius_w.value(); }
	const float radiusT() const { return _radius_w.targetValue(); }
	void setRadius(float radius) { _radius_w.setTarget(radius); }

private:
	Warping<qreal> _radius_w;
	Warping<QPointF> _pos_w;
	QSizeF _size;

};

class ClutchHandle : public Handle, public Timed
{
public:
	ClutchHandle(const QPointF & pos, const MagnifyingGlass * mg );
	~ClutchHandle();
	float cdRatio() const { return _mg->ratio(); }
	bool isAbsolute() const { return false; }
	void setPos(const QPointF & pos);
	const QPointF & posT() const { return _mg->dstCenterT(); }
	void setSrcPos(const QPointF & pos) { _mg->setSrcCenter(pos); }
	void setSrcRadius(float radius) { _original_src_radius = radius; updateSrcRadius(); }
	float srcRadiusT() const { return _mg->srcRadiusT(); }
	const MagnifyingGlass * magnifyingGlass() const { Q_ASSERT(!isAbsolute()); return _mg; }
	void setDstRadius(float radius) { _mg->setDstRadius(radius, true); }
	float radiusT() const { return _mg->dstRadiusT(); }
	void goBackIntoBounds(float aspect_ratio);

	void timeStep();
	bool finished() const;

	void optimizePosition(float aspect_ratio);

	enum RadiusMode
	{
		OriginalRadius,
		SmallRadius
	};
	void setRadiusMode(RadiusMode mode);
	void updateSrcRadius();
	void updateDstRadius();

private:
	MagnifyingGlass * _mg;
	float _original_dst_radius;
	float _original_src_radius;
	RadiusMode _radius_mode;

};

#endif // handle_h__