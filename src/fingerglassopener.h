#ifndef fingerglassopener_h__
#define fingerglassopener_h__

#include "scenetouchpoint.h"
#include "touchpointmanager.h"

class FingerGlassOpener : public QObject
{
	Q_OBJECT
public:
	FingerGlassOpener(TouchPointManager * tpm, int msecs);

	// touch management
	void addTouchById(int id);
	void removeTouchById(int id);
	bool hasTouchById(int id) const;
	bool hasTouches() const;

public slots:
	void timeOut();

signals:
	void openerTimedOut(const QList<SceneTouchPoint *> & touches);

private:
	QMap<float, int> _touches;
	QTimer _timer;
	int _msecs;
	TouchPointManager * _tpm;

};


#endif // fingerglassopener_h__