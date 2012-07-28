#include <QtGui>
#include "glwidget.h"
#include "mainwindow.h"
#include "timesingleton.h"
#include "studycreator.h"

//#include <QtOpenGL>
#include <QtDebug>

MainWindow::MainWindow()
{
	//this->setStyleSheet("background-color: black;");

    centralWidget = new QWidget(this);
	//centralWidget->setBackgroundRole(QPalette::Text);
    setCentralWidget(centralWidget);

	centralLayout = new SquareItemWithSidePaneLayout;



	QTimer *timer = new QTimer(this);
	//connect(timer, SIGNAL(timeout()), this, SLOT(timeStep()));
	//connect(timer, SIGNAL(timeout()), TimeSingleton::instance(), SLOT(timeStep()));
	connect(timer, SIGNAL(timeout()), this, SLOT(timeStep()));

#ifdef USE_NETWORK_CLIENT
  connect(timer, SIGNAL(timeout()), &raw_touch_adapter, SLOT(generateTouchEvent()));
	connect(&raw_touch_adapter, SIGNAL(touchEvent(QTouchEvent *)), this, SLOT(touchEvent(QTouchEvent *)));
#endif
	timer->start(10);

    createActions();
    createMenus();

	
	//setBackgroundRole(QPalette::Text);
	//QSpacerItem * spacerItem = new QSpacerItem(100, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
	//centralLayout->addItem(spacerItem, 0, 0);


	_side_pane = new SidePane(centralWidget);
	centralLayout->addWidget(_side_pane);

	//QGridLayout * sidepane_layout = new QGridLayout;
	//QPushButton * btn = new QPushButton("blah", sidepane);
	//QLabel * lbl = new QLabel("asdf", sidepane);
	//QListWidget * lwg = new QListWidget(sidepane);
	//lwg->insertItem(0, "blah");
	//lwg->insertItem(0, "ablah");
	//lwg->setEnabled(false);
	//btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//lwg->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	//QFont fnt("Myriad Pro", 24, QFont::Bold);
	//lbl->setFont(fnt );
	//sidepane_layout->addWidget(btn);
	//sidepane_layout->addWidget(lbl);
	//sidepane_layout->addWidget(lwg);
	//sidepane->setLayout(sidepane_layout);

	//QSpacerItem * spacerItem2 = new QSpacerItem(100, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
	//centralLayout->addItem(spacerItem2, 0, 2);

	//QPushButton *pushButton = new QPushButton("blah");
	//pushButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	//centralLayout->addWidget(pushButton, 0, 1);


    setWindowTitle(tr("Fat Finger Playground"));
    //resize(1280, 720);
	resize(1200,900);


	connect(_side_pane, SIGNAL(highlightCurrentBall()), this, SLOT(highlightCurrentBall()));
	connect(_side_pane, SIGNAL(changeStudy(QString)), this, SLOT(changeStudy(QString)));

	move(-1500,100);
	//glWidget->start();

	_study = NULL;
	_gl_widget = NULL;

  changeStudy("[interaction] fingerglass normal");

}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About FatFinger"),
            tr("<b>FatFinger</b> deals with fat fingers."));
}

void MainWindow::createActions()
{
    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exitAct);
 
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}

// --------------------------------------------------------
bool MainWindow::event( QEvent * event )
{
	QTouchEvent * touch_event;
	if(touch_event = dynamic_cast<QTouchEvent *>(event))qDebug() << "y" << time.restart();
	if(touch_event = dynamic_cast<QTouchEvent *>(event))
	{
		_gl_widget->touchEvent(touch_event);
	}
	if(touch_event = dynamic_cast<QTouchEvent *>(event))qDebug() << "z" << time.restart();
	bool result = QMainWindow::event(event);

	if(touch_event = dynamic_cast<QTouchEvent *>(event))qDebug() << "a" << time.restart();
	QCoreApplication::removePostedEvents(this, QEvent::TouchBegin);
	if(touch_event = dynamic_cast<QTouchEvent *>(event))qDebug() << "b" << time.restart();
	QCoreApplication::removePostedEvents(this, QEvent::TouchUpdate);
	if(touch_event = dynamic_cast<QTouchEvent *>(event))qDebug() << "c" << time.restart();
	return result;
}

// --------------------------------------------------------
#ifdef USE_NETWORK_CLIENT
void MainWindow::updateContacts( const ContactSet<FwContact> * set )
{
	raw_touch_adapter.updateContacts(set);
}
#endif

// --------------------------------------------------------
void MainWindow::timeStep()
{
	TimeSingleton::instance()->timeStep();
	//if(!TimeSingleton::instance()->hasSubscribers())
	//{
	//qDebug() << "asking for repaint";
	if(_gl_widget != NULL)
	{
		_gl_widget->repaint();
	}
	//}
}

// --------------------------------------------------------
void MainWindow::touchEvent( QTouchEvent * event )
{
	QString type;
	if(event->type() == QEvent::TouchBegin) type = "begin";
	if(event->type() == QEvent::TouchUpdate) type = "update";
	if(event->type() == QEvent::TouchEnd) type = "end";

	QString contains;
	if(event->touchPointStates() & Qt::TouchPointPressed) contains += "pressed ";
	if(event->touchPointStates() & Qt::TouchPointMoved) contains += "moved ";
	if(event->touchPointStates() & Qt::TouchPointReleased) contains += "released ";
	if(event->touchPointStates() & Qt::TouchPointStationary) contains += "stationary ";

//	qDebug() << type << event->touchPoints().size() << "containing" << contains;
#ifdef USE_NETWORK_CLIENT
	_gl_widget->touchEvent(event);
#endif
	
	//_study->update();
	if(_study->currentTask() != NULL && !_study->currentTask()->isActive())
	{
		_gl_widget->clearTouchWidgets();
		_study->nextTask();
		if(_study->currentTask() != NULL)
		{
			_gl_widget->setTask(_study->currentTask());
			_gl_widget->startTask();
		}
		else
		{
			//QCoreApplication::removePostedEvents(glWidget);
			//delete glWidget;
			//glWidget = NULL;
			_gl_widget->deleteLater();
			//glWidget = NULL;
			//glWidget->setTask(NULL);
			//glWidget->createTouchDisplayWidget();
		}
	}
	//qDebug() << typeid(_study->currentTask()).name();
}

// --------------------------------------------------------
void MainWindow::setStudy( Study * study )
{
	Q_ASSERT(study != NULL);
	this->_study = study;
	_gl_widget->setTask(study->currentTask());
	_gl_widget->startTask();
	//glWidget->createTouchDisplayWidget();
}

// --------------------------------------------------------
void MainWindow::highlightCurrentBall()
{
	_study->currentTask()->highlightCurrentBall();
}

// --------------------------------------------------------
void MainWindow::changeStudy( QString name )
{
	//qDebug() << "changing study to" << name;
	StudyCreator study_creator;
	Study * study = study_creator.createFromXML(QString("studies/%1.xml").arg(name));

	if(_gl_widget == NULL)
	{
		createGlWidget();
	}
	else
	{
		_gl_widget->clearTouchWidgets();
	}
	
	//if(_side_pane->selectedStudy() != name)
	//{
	//	_side_pane->selectStudy(name);
	//}
	if(this->_study != NULL)
	{
		delete this->_study;
		this->_study = NULL;
	}
	setStudy(study);
}

// --------------------------------------------------------
void MainWindow::glWidgetDestroyed()
{
	_gl_widget = NULL;
	

	//QFont fnt("Myriad Pro", 128, QFont::Bold);
	//QLabel * lbl = new QLabel("Study Complete!", centralWidget);
	//lbl->setFont(fnt);
	//lbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	//lbl->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	//centralLayout->addWidget(lbl);
}

// --------------------------------------------------------
void MainWindow::createGlWidget()
{
	_gl_widget = new GLWidget(centralWidget);
	QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Preferred);
	sp.setHeightForWidth(true);
	_gl_widget->setSizePolicy(sp);

	centralLayout->addWidget(_gl_widget);

	connect(_gl_widget, SIGNAL(sendSidePaneEvent(SidePaneEvent *)),
		_side_pane, SLOT(sidePaneEvent(SidePaneEvent *)));
	connect(_gl_widget, SIGNAL(destroyed()), this, SLOT(glWidgetDestroyed()));

#ifndef USE_NETWORK_CLIENT
	connect(_gl_widget, SIGNAL(sendTouchEventToMainWindow(QTouchEvent *)), this, SLOT(touchEvent(QTouchEvent *)));
#endif

	centralWidget->setLayout(centralLayout);

}
//
//// --------------------------------------------------------
//void MainWindow::mouseMoveEvent( QMouseEvent *event )
//{
////	qDebug() << "mousemove";
//	timeStep();
//}