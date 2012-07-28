#include "rawtouchadapter.h"

#include <QtDebug>
#include <cmath>

// --------------------------------------------------------
void RawTouchAdapter::updateContacts(const ContactSet<FwContact> * set)
{
	current_contacts_mutex.lock();
	foreach(FwContact * contact, current_contacts)
	{
		delete contact;
	}
	current_contacts.clear();
	std::vector<const FwContact *>::const_iterator it = set->getData().begin();
	while(it!=set->getData().end())
	{
		current_contacts[(*it)->getID()] = new FwContact(**it);
		++it;
	}
	current_contacts_mutex.unlock();
}

// --------------------------------------------------------
void RawTouchAdapter::FwContactToQTouchPoint( QTouchEvent::TouchPoint * point, const FwContact * contact )
{
	float radius = sqrt(contact->getArea()/3.1415f);
	radius *= 2; // HACK, as long as our method is indirect
	point->setRect(QRectF(-radius, -radius, +radius, +radius));
	point->setPos(QPointF(contact->getX(), contact->getY()));
	point->setId(contact->getID());
}

// --------------------------------------------------------
void RawTouchAdapter::generateTouchEvent()
{
	// TODO: detect stationary points. if all points are stationary,
	// then don't create any event.

	if(current_contacts.empty() && last_contacts.empty())
	{
		return;
	}
	current_contacts_mutex.lock();


	// determine event type based on comparing current and last
	// contact frame.
	QEvent::Type qevent_type = QEvent::TouchUpdate;
	//qDebug() << "cur:" << current_contacts.size() << "las:" << last_contacts.size();
	if(current_contacts.size()>0 && last_contacts.size()==0)
		qevent_type = QEvent::TouchBegin;
	if(current_contacts.size()==0 && last_contacts.size()>0)
		qevent_type = QEvent::TouchEnd;


	// potentially all touches of the last contact frame could be
	// released in this frame. those which are not are going to be
	// eliminated later.
	QSet<int> release_candidates;
	foreach(FwContact * c, last_contacts)
	{
		release_candidates.insert(c->getID());
	}


	// prepare some of the necessary variables for a QTouchEvent.
	Qt::TouchPointStates qevent_touchstates = 0;
	QList<QTouchEvent::TouchPoint> qevent_touchlist;


	int cnt=0; int cnt2=0;
	foreach(FwContact * c, current_contacts) { cnt2++; if(c==0) ++cnt; }

	// now compare all of the previous contacts to the current ones.
	//for(int i=0; i<current_contacts.size(); ++i)
	foreach(const FwContact * c, current_contacts)
	{
		// prepare data structures of both Qt and Mtnet type
		//FwContact * blah = (current_contacts[i]);
		const FwContact & contact = *c;
		QTouchEvent::TouchPoint point;
		FwContactToQTouchPoint(&point, &contact);

		
		// preliminarily determine touch point state. because it is currently
		// pressed, it can only be moved or pressed (not released).
		Qt::TouchPointStates state;
		if(last_contacts.contains(contact.getID()))
		{
			//point.setStartPos();
			//point.setStartPos(last_contacts)
			release_candidates.remove(contact.getID());

			//delete last_contacts[contact.getID()];
			state = Qt::TouchPointMoved;
		}
		else
		{
			state = Qt::TouchPointPressed;
			starting_positions[contact.getID()] = point.pos();

			// if a not-yet-pressed contact is release candiate, then
			// something would be wrong.
			Q_ASSERT(!release_candidates.contains(contact.getID()));
		}

		Q_ASSERT(starting_positions.contains(contact.getID()));
		point.setStartPos(starting_positions.value(contact.getID()));

		// ???
		//last_contacts[contact.getID()] = new FwContact(contact);


		// store new Qt touch point into the list for the event
		point.setState(state);
		qevent_touchlist.push_back(point);
		qevent_touchstates |= state;
	}

	foreach(int id, release_candidates)
	{
		QTouchEvent::TouchPoint point;// = new QTouchEvent::TouchPoint();
		FwContactToQTouchPoint(&point, last_contacts[id]);
		Q_ASSERT(starting_positions.contains(id));
		point.setStartPos(starting_positions.value(id));
		point.setState(Qt::TouchPointReleased);

		qevent_touchlist.push_back(point);
		delete last_contacts[id];
		last_contacts.remove(id);
		starting_positions.remove(id);

		qevent_touchstates |= Qt::TouchPointReleased;
	}

	//qDebug() << "rel can size " << release_candidates.size() << "cur size " << last_contacts.size() << "qev size" << qevent_touchlist.size();
	//qDebug() << "posting event of type" << qevent_type;
	QTouchEvent * qevent = new QTouchEvent(qevent_type, QTouchEvent::TouchScreen,
		Qt::NoModifier, qevent_touchstates, qevent_touchlist);
	//++eventnum;
	//	if(eventnum%10==0)
	{
		//QApplication::postEvent(main_win, qevent, eventnum);
		//QApplication::processEvents();
		//QApplication::sendPostedEvents(main_win, qevent_type);
		//main_win->updateContacts(last_contacts);
	}
	//	old_set.replaceWith(*set);
	//qDebug() << "process" << time.elapsed();



	// clean up
	cloneMap(&last_contacts, &current_contacts);
	current_contacts_mutex.unlock();

	//glWidget->setTest2(pt);

	//QTouchEvent * qevent = 0; // todo
	emit touchEvent(qevent);
	delete qevent; // 
}


// --------------------------------------------------------
template<typename K, typename V>
void RawTouchAdapter::cloneMap( QMap<K,V *> * dst, const QMap<K,V *> * src )
{
	foreach(V * v, *dst)
	{
		delete v;
	}
	dst->clear();
	foreach(const K & k, src->keys())
	{
		(*dst)[k] = new V(*(*src)[k]);
	}
}

