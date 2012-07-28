#include "glwidget.h"
#include "magnifyingwidgetcontroller.h"
#include "magnifyingwidgetrenderer.h"
#include "rigidwidgetcontroller.h"
#include "touchdisplaywidgetrenderer.h"
#include "texturedrigidwidgetrenderer.h"
#include "handlewidgetcontroller.h"
#include "handlewidgetrenderer.h"
#include "pointwidgetconnection.h"
#include <QtGui>
#include <cmath>
#include "selectiontask.h"
#include "fingerglasstool.h"
#include "directtouchtool.h"
#include <QGLFrameBufferObject>

// ------------------------------------------------------------------------
GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
{
    setAutoFillBackground(false);
	setAttribute(Qt::WA_AcceptTouchEvents);
	setAttribute(Qt::WA_TouchPadAcceptSingleTouchEvents);
    makeCurrent();

	this->_tpm = new TouchPointManager();
	this->_twm = new TouchWidgetManager(this);
	this->_current_task = NULL;
	this->_resource_container = new GLResourceContainer(context());

	// initialize resource container
	connect(_resource_container, SIGNAL(requestGLResource(GLResourceType, const QString &)),
		this, SLOT(requestGLResource(GLResourceType, const QString &)));
}


// --------------------------------------------------------
void GLWidget::startTask()
{
	_current_task->setup();
	//createTouchDisplayWidget();
	_current_task->start();
}

// ------------------------------------------------------------------------
GLWidget::~GLWidget()
{
	delete _tpm;
	delete _twm;
	delete _resource_container;
}

// ------------------------------------------------------------------------
void GLWidget::timeStep()
{
	// TODO, maybe for fancy gfx stuff
}

// ------------------------------------------------------------------------
void GLWidget::initializeGL()
{
  GLenum err = glewInit();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// ------------------------------------------------------------------------
void GLWidget::paintGL()
{
	//if(_frame_draw_time == NULL)
	//{
	//	_frame_draw_time = new QTime();
	//	_frame_draw_time->start();
	//}
	//else
	//{
	//	qDebug() << "elapsed since last dra: "
	//}
	QTime frame_draw_time;
	frame_draw_time.start();

	// setup
	qglClearColor(Qt::black);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	setupViewport();

	// draw the widgets
	drawWidgets();

	//qDebug() << "drawing took" << frame_draw_time.elapsed();
}

// --------------------------------------------------------
void GLWidget::drawWidgets()
{
	QObject * sender = QObject::sender();
	//bool omit_scene_drawing_widgets = false;
	TouchWidgetRenderer * to_texture_renderer = NULL;
	if(sender != NULL)
	{	
		to_texture_renderer = dynamic_cast<TouchWidgetRenderer *>(sender);
		//Q_ASSERT(dynamic_cast<TouchWidgetRenderer *>(sender)->isSceneDrawer());
		//Q_ASSERT(->isSceneDrawer());
		Q_ASSERT(to_texture_renderer != NULL);
		//omit_scene_drawing_widgets = true;
	}

	//TouchWidgetMultiMap::const_iterator tw_it = _twm->widgets().begin();
	//while(tw_it != _twm->widgets().end())
	//{
	//	// avoid infinite recursion in a render-to-texture mode
	//	if( omit_scene_drawing_widgets && // we are in render-to-texture mode
	//		(*tw_it)->renderer()->isSceneDrawer() ) // renderer would like to do render-to-texture stuff
	//	{
	//		break;
	//	}
	//	(*tw_it)->renderer()->paint(_resource_container);
	//	++tw_it;
	//}
	//_twm->paintAll(_resource_container, omit_scene_drawing_widgets);
	_twm->paintAll(_resource_container, to_texture_renderer);
}


// ------------------------------------------------------------------------
void GLWidget::resizeGL(int width, int height)
{
	width; // stop compiler complaint
	height; // stop compiler complaint
	_tpm->setSize(QSizeF(width, height));
	setupViewport();
	//twm->setScreenSize(this->size());
}

// ------------------------------------------------------------------------
void GLWidget::setupViewport()
{
    glViewport(0, 0, width(), height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	float aspect_ratio = (float)width()/height();
	float margin = (1.0f-(1.0f/aspect_ratio))/2.0f;
    glOrtho(0.0, 1.0, 1.0-margin, margin, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ------------------------------------------------------------------------
void GLWidget::mousePressEvent(QMouseEvent *event)
{
    emit targetClicked(event->pos());
    last_mouse_pos = normalizedCoords(event->posF());
}

// ------------------------------------------------------------------------
void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    last_mouse_pos = normalizedCoords(event->posF());
}

// ------------------------------------------------------------------------
QPointF GLWidget::normalizedCoords(QPointF coords)
{
    return QPointF(coords.x()/this->width(), coords.y()/this->height());
}

// --------------------------------------------------------
void GLWidget::touchEvent( QTouchEvent * touch_event )
{
	//// do coordinate transform
	//QList<QTouchEvent::TouchPoint>::iterator it = touch_event->touchPoints().begin();
	//while(it != touch_event->touchPoints().end())
	//{
	//	QPointF orig_p = it->pos();
	//	QPointF new_p;
	//	it->setRect(QRectF(0.0f, 0.0f, 0.01f, 0.01f));
	//	new_p.setX(orig_p.x()/width());
	//	new_p.setY(orig_p.y()/height());
	//	it->setPos(new_p);
	//}

	_tpm->touchEvent(touch_event, _twm);
	emit sendTouchEventToMainWindow(touch_event);
	Q_ASSERT(debug_touchInvariantHolds());
	//repaint();
}


// --------------------------------------------------------
void GLWidget::requestGLResource( GLResourceType type, const QString & name )
{
	switch(type)
	{
	case GLTexture:
		{
			QPixmap * pm;

			// first, check if the texture is available directly in the file system.
			// otherwise, load it from the resource file. if it exists in neither
			// of those locations, throw an assertion violation.
			QString direct_file_name = QString("%1.png").arg(name);
			QFile direct_file(direct_file_name);
			if(direct_file.exists())
			{
				pm = new QPixmap(direct_file_name);				
			}
			else
			{
				pm = new QPixmap(QString(":/textures/%1.png").arg(name));
			}
			Q_ASSERT(!pm->isNull());
			GLuint tex = bindTexture(*pm, GL_TEXTURE_2D, GL_RGBA,
				QGLContext::MipmapBindOption | QGLContext::LinearFilteringBindOption);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			_resource_container->addTexture(name, tex);
			break;
		}
	case GLShaderProgram:
		{
			QGLShaderProgram * p = new QGLShaderProgram();
			if(!p->addShaderFromSourceFile(QGLShader::Vertex, QString(":/shader/%1.vert").arg(name)))
			{
				qDebug() << "Vertex Shader Compile Error:";
				qDebug() << p->log();
			}
			if(!p->addShaderFromSourceFile(QGLShader::Fragment, QString(":/shader/%1.frag").arg(name)))
			{
				qDebug() << "Fragment Shader Compile Error:";
				qDebug() << p->log();
			}
			if(!p->link())
			{
				qDebug() << "Shader Program Linker Error:";
				qDebug() << p->log();
			}
			Q_ASSERT(p->isLinked());
			_resource_container->addShaderProgram(name, p);
			break;
		}
	case GLFramebufferObject:
		{
			//QGLFramebufferObject * fbo = new QGLFramebufferObject(256, 256);
			
			QGLFramebufferObject * fbo = new QGLFramebufferObject(1024, 1024);
			glBindTexture(GL_TEXTURE_2D, fbo->texture());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);
			Q_ASSERT(fbo->isValid());
			_resource_container->addFramebufferObject(name, fbo);
			break;
		}
	case GLVertexBufferObject:
		{
			GLuint vbo;
			glGenBuffers(1, &vbo);
			_resource_container->addVertexBufferObject(name, vbo);
			break;
		}
	default:
		Q_ASSERT(false);
	}
}


// --------------------------------------------------------
void GLWidget::sidePaneEvent( SidePaneEvent * event )
{
	emit sendSidePaneEvent(event);
}

// --------------------------------------------------------
void GLWidget::setTask( Task * task )
{
	//Q_ASSERT(task != NULL);
	if(task != NULL)
	{
		if(_current_task != NULL)
		{
			disconnect(_current_task);
			_tpm->removeTouchListener(_current_task);
		}
		task->scene()->setTouchWidgetManager(_twm);
		_twm->setSelectionMode(task->settingContainer()->stringProperty("selection_mode"));
		//_current_task->setupScene(_twm);

		// in order for the create[...]IfNecessary functions to work
		foreach(Tool * tool, *task->tools())
		{
			tool->setTouchPointManager(_tpm);
			tool->setTouchWidgetManager(_twm);
		}

		// assign tools
		_tpm->setToolList(task->tools());
		//_tpm->setUpdateClosestWidgetsFlag(_twm->selectionMode() == BubbleSelection);

		connect(task, SIGNAL(sendSidePaneEvent(SidePaneEvent *)),
			this, SLOT(sidePaneEvent(SidePaneEvent *)));
		_tpm->addTouchListener(task);
	}

	_current_task = task;
}

// --------------------------------------------------------
void GLWidget::clearTouchWidgets()
{
	_twm->clear();
}

// --------------------------------------------------------
void GLWidget::createTouchDisplayWidget()
{
	//RigidWidgetController * rwc = new RigidWidgetController(TouchDisplayWidgetType);
	//TouchDisplayWidgetRenderer * tdwr = new TouchDisplayWidgetRenderer(rwc, _tpm);
	//TouchWidget * tw = new TouchWidget(rwc, tdwr);
	//qDebug() << "created" << tw;
	//_twm->add(tw);	
}

// --------------------------------------------------------
bool GLWidget::debug_touchInvariantHolds()
{
	// check invariant: a touch T is in the key list of widget W, iff
	// T is owned by A

	foreach(const TouchWidget * tw, _twm->widgets())
	{
		foreach(const PointWidgetConnection * const pwc, tw->controller()->touchPointConnections())
		{
			//Q_ASSERT(pwc->point()->widgetConnections().contains(const_cast<PointWidgetConnection *>(pwc)));
			if(!pwc->point()->widgetConnections().contains(const_cast<PointWidgetConnection *>(pwc)))
			{
				return false;
			}
		}
		// SLOW
		//Q_ASSERT(twm->widgets().values().contains(const_cast<TouchWidget *>(tw)));
	}
	foreach(const SceneTouchPoint * stp, _tpm->touches())
	{
		foreach(const PointWidgetConnection * pwc, stp->widgetConnections())
		{
			if(!pwc->widgetController()->touchPointConnections().values().contains(const_cast<PointWidgetConnection *>(pwc)))
			{
				return false;
			}
		}
		if(!_tpm->touches().contains(stp->id()))
		{
			return false;
		}
	}
	return true;
}

// --------------------------------------------------------
bool GLWidget::event( QEvent * event )
{	
	QTouchEvent * touch_event = dynamic_cast<QTouchEvent *>(event);
	if(touch_event != NULL)
	{
		touchEvent(touch_event);
		return true;
	}
	else
	{
		return QWidget::event(event);
	}
}
