#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <GL/glew.h>
#include "glresourcecontainer.h"
#include "touchwidgetmanager.h"
#include "task.h"
#include "tool.h"

//#include <QtOpenGL>
#include <QGLWidget>
#include <QGraphicsScene>
#include <QGLShaderProgram>
#include <QTouchEvent>

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
	~GLWidget();
	void startTask();
	void touchEvent(QTouchEvent * touch_event); 
	bool event(QEvent * event);

	void setTask(Task * task);
	void clearTouchWidgets();
	void createTouchDisplayWidget();

public slots:
	void drawWidgets();
	void setupViewport();
	void requestGLResource(GLResourceType type, const QString & name);
	void sidePaneEvent(SidePaneEvent * event);

signals:
    void targetClicked(QPoint pos);
	void sendSidePaneEvent(SidePaneEvent * event);
	void sendTouchEventToMainWindow(QTouchEvent * event);

protected:
	// drawing stuff
    void initializeGL();
    void paintGL();
	void resizeGL(int width, int height);

	// mouse event stuff
	void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    QPointF normalizedCoords(QPointF coords);

	// debug code
	bool debug_touchInvariantHolds();

protected slots:
    void timeStep();

private:
    QPointF last_mouse_pos;

	TouchPointManager * _tpm;
	TouchWidgetManager * _twm;
	GLResourceContainer * _resource_container;

	Task * _current_task;

	//QTime * _frame_draw_time;
};

#endif
