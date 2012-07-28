#ifndef timesingleton_h__
#define timesingleton_h__

#include <QSet>
#include <QObject>
#include "timed.h"

class TimeSingleton : public QObject
{
	Q_OBJECT

public:
	//TimeSingleton();
	static TimeSingleton * instance();
	void subscribe(Timed * obj);
	void unsubscribe(Timed * obj);
	bool isSubscribed(Timed * obj);
	bool hasSubscribers() const;

public slots:
	void timeStep();

private:
	static TimeSingleton * _instance;
	QSet<Timed *> _subscribers;
	QSet<Timed *> _removed_during_iteration;
};

#endif // timesingleton_h__