#ifndef task_h__
#define task_h__

#include "taskstatistics.h"
#include "scenetouchpoint.h"
#include "touchwidgetmanager.h"
#include "touchpointmanager.h"
#include "sidepaneevent.h"
#include "settingcontainer.h"
#include "scene.h"
#include "tool.h"
#include "touchlistener.h"

class Task : public QObject, public TouchListener
{
	Q_OBJECT
public:
	Task(SettingContainer * node);
	~Task();
	virtual QString caption() const = 0;
	virtual QString prefix() const = 0;
	virtual void startRound() = 0;
	virtual void stopRound() = 0;
	int numRounds() const
	{
		//return settingContainer()->intProperty("num_repetitions");
		return settingContainer()->intProperty("num_distances") * settingContainer()->intProperty("num_blocks");
	}
	virtual void highlightCurrentBall() = 0;
	void nextRound(bool same);
	int round() const { return _round; }
	bool isActive() const { return _round <= numRounds(); }
	void addTool(Tool * tool) { this->_tools << tool; }
	void start();
	void setScene(Scene * scene) { _scene = scene; }
	QList<Tool *> * tools() { return &this->_tools; }
	void setup();
	Scene * scene() { return _scene; }
	const SettingContainer * settingContainer() const { return _setting_container; }

signals:
	void sendSidePaneEvent(SidePaneEvent * event);

protected:
	const Scene * scene() const { return _scene; }
	TaskStatistics * taskStatistics() { return _task_statistics; }

private:
	SettingContainer * _setting_container;
	QList<Tool *> _tools;
	Scene * _scene;
	int _round;
	TaskStatistics * _task_statistics;

	friend QDebug operator<<(QDebug dbg, const Task &t);
};


#endif // task_h__