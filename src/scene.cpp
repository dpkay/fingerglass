#include "scene.h"
#include "texturedrigidwidgetrenderer.h"
#include "touchdisplaywidgetrenderer.h"
#include "variousattachments.h"

#include <QCryptographicHash>
#include <QDir>

// --------------------------------------------------------
QDebug operator<<(QDebug dbg, const Scene &s)
{
	dbg.nospace() << "\t\t" << &s << "has the following settings:\n" << *s._setting_container;
	return dbg.space();
}

// --------------------------------------------------------
Scene::Scene( SettingContainer * setting_container )
{
	this->_setting_container = setting_container;
	this->_twm = NULL;
	float ball_margin = _setting_container->floatProperty("ball_margin");
	float vertical_margin = (1.0f-(1.0f/_setting_container->floatProperty("aspect_ratio")))/2.0f;
	_scene_rect = QRectF(ball_margin, vertical_margin+ball_margin, 1-2*ball_margin, 1.0f-2*vertical_margin-2*ball_margin);
}


// --------------------------------------------------------
void Scene::createBackground()
{
	if(_setting_container->hasStringProperty("background"))
	{
		RigidWidgetController * rwc = new RigidWidgetController(MapWidgetType);
		rwc->setPos(QPointF(0.5f, 0.5f));
		rwc->setRadius(0.5f);
		rwc->setMovable(false);
		TexturedRigidWidgetRenderer * trwr = new TexturedRigidWidgetRenderer(rwc, _setting_container->stringProperty("background"), false, false);
		TouchWidget * tw = new TouchWidget(rwc, trwr);
		_twm->add(tw);
	}
}

// --------------------------------------------------------
void Scene::createBalls()
{
	int num_points = _setting_container->intProperty("num_points");
	QImage * mask = NULL;
	if(_setting_container->hasStringProperty("mask"))
	{
		mask = new QImage(QString(":/textures/%1.png").arg(_setting_container->stringProperty("mask")));
	}


	QByteArray ba;
	QDataStream ds(&ba, QIODevice::WriteOnly);
	_setting_container->serialize(&ds);
    QByteArray hash = QCryptographicHash::hash(ba, QCryptographicHash::Md5);
    QDir("cache").mkpath(".");
	QString file_name = QString("cache/%1.scenecache").arg(QString(hash.toHex()));
	QFile file(file_name);

	if(file.exists())
	{
		if(!file.open(QIODevice::ReadOnly))
		{
			Q_ASSERT(false);
		}
		//point_stream = new QDataStream(&file, QIODevice::ReadOnly);
	}
	else
	{
		if(!file.open(QIODevice::WriteOnly))
		{
			Q_ASSERT(false);
		}
		//point_stream = new QDataStream(&file, QIODevice::WriteOnly);
	}
	//Q_ASSERT(point_stream != NULL);
	QDataStream point_stream(&file);

	//QDataStream out(&file);

	//qDebug() << point_stream.device()->openMode();
	

	for(int i=0; i<num_points; ++i)
	{
		QPointF pos;
		if(point_stream.device()->openMode() == QIODevice::WriteOnly)
		{
			int mask_value;
			do
			{
				pos = randomRectPos(_scene_rect);
				if(mask != NULL)
				{
					int x = (int) (pos.x()*mask->width());
					int y = (int) (pos.y()*mask->height());
					mask_value = mask->pixel(x,y);
				}
			}
			while((mask != NULL && mask_value != -1) || isOverlappingWithAnyBall(pos));
			point_stream << pos;
			//qDebug() << "pos" << pos;
		}
		else
		{
			point_stream >> pos;
		}

		
		RigidWidgetController * rwc = new RigidWidgetController(WaypointWidgetType);
		rwc->setPos(pos);
		rwc->setRadius(_setting_container->floatProperty("point_radius"));
		//rwc->setSelectableByBubbleOnly(_setting_container->boolProperty("bubble_exclusive"));
		rwc->setSelectionMode(_twm->selectionMode());
		TexturedRigidWidgetRenderer * trwr = new TexturedRigidWidgetRenderer(rwc, "button", true, true);
		TouchWidget * tw = new TouchWidget(rwc, trwr);
		_twm->add(tw);
		_balls << rwc;
	}

	file.close();
}


// --------------------------------------------------------
bool Scene::isOverlappingWithAnyBall( const QPointF & pos ) const
{
	bool overlapping = false;
	foreach(const RigidWidgetController * rwc, _balls)
	{
		if(QLineF(rwc->pos(), pos).length() < 2*rwc->coreRadius())
		{
			overlapping = true;
			break;
		}
	}
	return overlapping;
}

// --------------------------------------------------------
const QList<RigidWidgetController *> & Scene::balls() const
{
	return _balls;
}

// --------------------------------------------------------
RigidWidgetController * Scene::randomBall() const
{
	return _balls.value(qrand() % _balls.size());
}

// --------------------------------------------------------
QPointF Scene::randomRectPos( const QRectF & range )
{
	return QPointF(
		range.left() + (float)qrand()/RAND_MAX * range.width(),
		range.top() + (float)qrand()/RAND_MAX * range.height()
		);
}

// --------------------------------------------------------
void Scene::setup()
{
	Q_ASSERT(_twm != NULL);
	createBackground();
	createBalls();
}

// --------------------------------------------------------
void Scene::setTouchWidgetManager( TouchWidgetManager * twm )
{
	_twm = twm;
}

// --------------------------------------------------------
RigidWidgetController * Scene::createTarget( const QPointF & center )
{
	RigidWidgetController * rwc = new RigidWidgetController(TargetWidgetType);
	/*QPointF pos;
	do 
	{
		pos = randomRingPos(
			center,
			settingContainer()->floatProperty("min_target_distance"),
			settingContainer()->floatProperty("max_target_distance")
		);
	}
	while(isOverlappingWithAnyBall(pos));*/
	rwc->setPos(center);
	rwc->setMovable(false);
	rwc->setRadius(_setting_container->floatProperty("target_radius"));
	TexturedRigidWidgetRenderer * trwr = new TexturedRigidWidgetRenderer(rwc, "target", false, false);
	TouchWidget * tw = new TouchWidget(rwc, trwr);
	_twm->add(tw);
	return rwc;
}

// --------------------------------------------------------
void Scene::remove( RigidWidgetController * rwc )
{
	_twm->remove(rwc);
}

// --------------------------------------------------------
QPointF Scene::randomRingPos( const QPointF & center, float inner_radius, float outer_radius )
{
	QPointF pos;
	float length;
	//QRectF screen_rect()
	QRectF ring_rect(center - QPointF(outer_radius, outer_radius), center + QPointF(outer_radius, outer_radius));
	do
	{
		pos = randomRectPos(ring_rect);
		length = QLineF(pos, center).length();
	}
	while(length > outer_radius || length < inner_radius || !this->_scene_rect.contains(pos));
	return pos;
}

// --------------------------------------------------------
void Scene::createBallTargetPair( RigidWidgetController ** ball_rwc, RigidWidgetController ** target_rwc, int exponent )
{
	// determine a distance
	//static int exponent = -1;
	float distance;
	//++exponent%=8;
	distance = powf(2,exponent)*0.9f/128;

	// try finding a ball/target pair
	QPointF target_pos;
	do
	{
		*ball_rwc = randomBall();

		float angle = (float)qrand()/RAND_MAX*3.1415;
		target_pos = (*ball_rwc)->pos() + QPointF(cos(angle), sin(angle))*distance;

		//qDebug() << (*ball_rwc)->pos() << QPointF(cos(angle), sin(angle))*distance;
	}
	while(isOverlappingWithAnyBall(target_pos) || !this->_scene_rect.contains(target_pos));

	*target_rwc = createTarget(target_pos);

	_twm->setCurrentBallTarget(*ball_rwc, *target_rwc);

	qDebug() << "generated" << (*ball_rwc)->pos() << (*target_rwc)->pos();
}
