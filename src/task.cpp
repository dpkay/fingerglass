#include "task.h"

Task::Task(SettingContainer * setting_container) :
	_setting_container(setting_container)
{
	//this->_setting_container = setting_container;
	this->_task_statistics = new TaskStatistics(numRounds());
	this->_round = 0;
}

// --------------------------------------------------------
void Task::start()
{
	TaskCreationEvent * event = new TaskCreationEvent(caption(), prefix(), _task_statistics);
	emit sendSidePaneEvent(event);
	delete event;
	nextRound(false);
}

// --------------------------------------------------------
void Task::nextRound(bool same)
{
	Q_ASSERT(isActive());
	stopRound();
	if(!same)
	{
		++_round;
	}
	if(isActive())
	{
		startRound();
	}
	else
	{
		SidePaneEvent * spe = new TaskCompletionEvent;
		emit sendSidePaneEvent(spe);
		delete spe;
	}
}

// --------------------------------------------------------
void Task::setup()
{
	scene()->setup();
	foreach(Tool * tool, _tools)
	{
		tool->setup();
	}
}

// --------------------------------------------------------
Task::~Task()
{
	foreach(Tool * tool, _tools)
	{
		delete tool;
	}
	delete _scene;
	delete _task_statistics;
}


// --------------------------------------------------------
QDebug operator<<(QDebug dbg, const Task &t)
{
	dbg.nospace() << &t << "has the following tools:\n";
	foreach(const Tool * tool, t._tools)
	{
		dbg.nospace() << "\t\t" << *tool;
	}
	dbg.nospace() << "...and the following scene:\n" << *t._scene;
	dbg.nospace() << "...and the following settings:\n" << *t._setting_container;

	return dbg.space();
}