#ifndef widgetattachment_h__
#define widgetattachment_h__

class TouchWidgetRenderer;

#include "glresourcecontainer.h"
#include <QtCore>

class WidgetAttachment : public QObject
{
	Q_OBJECT
public:
	WidgetAttachment(QObject * twc, int closing_time = 0) :
		QObject(twc)
	{
		_age = NULL;
		_timer = NULL;
		if(closing_time != 0)
		{
			startClosingTimer(closing_time);
		}
	}

	void startClosingTimer(int lifetime)
	{
		Q_ASSERT(_age == NULL);
		Q_ASSERT(_timer == NULL);
		_lifetime = lifetime;
        _timer = new QTimer(this);
		_age = new QTime;
		_timer->setSingleShot(true);
		_timer->start(_lifetime);
		_age->start();
		connect(_timer, SIGNAL(timeout()), this, SLOT(timeOut()));
	}

	float lifeTime()
	{
		return (float)_age->elapsed()/_lifetime;
	}

public slots:
	void timeOut()
	{
		emit close();
	}

signals:
	void close();

private:
	QTime * _age;
	QTimer * _timer;
	int _lifetime;

};

#endif // widgetattachment_h__
