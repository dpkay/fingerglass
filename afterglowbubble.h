#ifndef afterglowbubble_h__
#define afterglowbubble_h__

#include <QTime>
#include <QRectF>

class AfterGlowBubble
{
public:
	AfterGlowBubble(QRectF * bubble_rect);
	~AfterGlowBubble();
	bool hasBubble() const;
	const QRectF & bubbleRect() const;
	const QTime & time() const;
	void moveTo(const QPointF & p);
	void restartTimer();
	bool expired() const;
	//bool contains(const QPointF & p) const;
	bool intersectsCircle(const QPointF & p, float radius) const;

	static void setLifeSpan(int span) { _life_span = span; }

private:
	QRectF * _bubble_rect;
	QTime _time;

	static int _life_span;

};

#endif // afterglowbubble_h__