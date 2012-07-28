#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMutex>
#include <QTime>
#include <QMainWindow>
#include <QMap>
#ifdef USE_NETWORK_CLIENT
#include "rawtouchadapter.h"
#endif
#include "study.h"
#include "sidepane.h"
#include "squareitemwithsidepanelayout.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
QT_END_NAMESPACE
class GLWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
	bool event(QEvent * event);
#ifdef USE_NETWORK_CLIENT
  void updateContacts(const ContactSet<FwContact> * set);
#endif
  void setStudy(Study * study);

public slots:
	void changeStudy(QString name);

private slots:
    void about();
	void timeStep();
	void touchEvent(QTouchEvent * event);

protected slots:
	void highlightCurrentBall();
	void glWidgetDestroyed();


private:
    void createActions();
    void createMenus();
	void createGlWidget();

    QWidget *centralWidget;
    GLWidget *_gl_widget;
	SidePane * _side_pane;

    QMenu *fileMenu;
    QMenu *helpMenu;
    QAction *exitAct;
    QAction *aboutAct;
	QTime time;
	Study *_study;

	SquareItemWithSidePaneLayout * centralLayout;

#ifdef USE_NETWORK_CLIENT
	RawTouchAdapter raw_touch_adapter;
#endif	

};

#endif

