#ifndef rawtouchadapter_h__
#define rawtouchadapter_h__

#include "config.h"

//#ifdef USE_NETWORK_CLIENT

#include <QTouchEvent>
#include <QMutex>

#include "MtNet\FwContact.h"
#include "MtNet\ContactSet.h"

class RawTouchAdapter : public QObject
{
Q_OBJECT

public:
	// this should be called by the server thread to store the most
	// recent contact frame. some of them may be ignored since the
	// most recent contact frame needs to be polled by generateTouchEvent()
	// in order to be processed.
	void updateContacts(const ContactSet<FwContact> * set);

public slots:
	// this should be called by the Qt application thread (e.g. using a timer),
	// polls the most recent contact frame and emits the touchEvent signal.
	void generateTouchEvent();

signals:
	// this signal should be connected to a handling function
	void touchEvent(QTouchEvent * event);

protected:
	// converts a FwContact into a Qt TouchPoint
	void FwContactToQTouchPoint( QTouchEvent::TouchPoint * point,
		const FwContact * contact );
	
	// clones a QMap including all its elements
	template<typename K, typename V>
		void cloneMap( QMap<K,V *> * dst, const QMap<K,V *> * src );

private:
	// most recently stored contact frame. this needs a mutex
	// since it is accessed by updateContacts (server thread) and
	// generateTouchEvent (Qt application thread)
	QMap<int, FwContact *> current_contacts;
	QMutex current_contacts_mutex;

	// contact frame that was most recently processed by generateTouchEvent()
	// in the Qt application thread. We need to compare this to
	// current_contacts in order to determine new/released touches.
	QMap<int, FwContact *> last_contacts;

	// starting positions of all current touches
	QMap<int, QPointF> starting_positions;
};

//#endif

#endif // rawtouchadapter_h__