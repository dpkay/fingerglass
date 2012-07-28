#include <QApplication>
#include <QtDebug>
#include <QTime>

// gui stuff
#include "mainwindow.h"

// multi-touch stuff
#ifdef USE_NETWORK_CLIENT
#include "MtNet/MtClient.h"
#include "MtNet/FwContact.h"
#endif

#include "studycreator.h"
 
MainWindow * main_win = NULL;

// sample contact event handler
//ContactSet<FwContact> old_set;


//int eventnum=0;
//QMap<int, FwContact *> current_contacts;
//#include <QTouchEvent>
//QTime ttime;
#ifdef USE_NETWORK_CLIENT
void processContacts(const ContactSet<FwContact> * set)
{ 
	if(main_win != NULL)
	{
		main_win->updateContacts(set);
	}
}
#endif

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(shader);
	Q_INIT_RESOURCE(texture);

	// initialize multi-touch client
#ifdef USE_NETWORK_CLIENT
	MtClient<FwContact> mt_client;
	mt_client.registerContactCallback(processContacts);
	if(!mt_client.connectTo("136.152.170.127"))
	//if(!mt_client.connectTo("10.0.0.3"))
	//if(!mt_client.connectTo("172.18.0.86"))
		return 1;
#endif USE_NETWORK_CLIENT

	// run gui application
	QPalette pal;
	pal.setColor(QPalette::Window, QColor(40,40,40));
	pal.setColor(QPalette::Base, QColor(70,70,70));
	pal.setColor(QPalette::WindowText, QColor(220,220,220));
	pal.setColor(QPalette::ButtonText, QColor(30,30,30));
	pal.setColor(QPalette::Text, QColor(240,240,240));
	QApplication::setPalette(pal);
	QApplication app(argc, argv);

	main_win = new MainWindow();

  //main_win->changeStudy("[interaction] fingerglass normal");
  //main_win->changeStudy("[freestyle] fingerglass normal");
	main_win->showMaximized();

    int result = app.exec();
	delete main_win;
	return result;
}
