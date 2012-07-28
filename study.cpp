#include "study.h"

QDebug operator<<(QDebug dbg, const Study &s)
{
	dbg.nospace() << "Study " << &s << " has the following tasks:\n";
	foreach(const Task * task, s.tasks())
	{
		dbg.nospace() << "\t" << *task;
	}

	return dbg.space();
}

// --------------------------------------------------------
Study::Study()
{
	_current_task_index = 0;
}

// --------------------------------------------------------
void Study::nextTask()
{
	++_current_task_index;
	//if(currentTask() != NULL)
	//{
	//	currentTask()->start();
	//}
}

// --------------------------------------------------------
Task * Study::currentTask()
{
	if(_current_task_index < _tasks.size())
	{
		return _tasks[_current_task_index];
	}
	else
	{
		return NULL;
	}
}

// --------------------------------------------------------
Study::~Study()
{
	foreach(Task * task, _tasks)
	{
		delete task;
	}
}
//// --------------------------------------------------------
//void Study::update()
//{
//	if(!currentTask()->isActive())
//	{
//		nextTask();
//	}
//}