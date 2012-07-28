#ifndef study_h__
#define study_h__

#include "task.h"

class Study
{
public:
	Study();
	~Study();
	void addTask(Task * task) { _tasks << task; }
	const QList<Task *> & tasks() const { return _tasks; }
	void nextTask();
	Task * currentTask();
	//void update();

private:
	int _current_task_index;
	QList<Task *> _tasks;
	friend QDebug operator<<(QDebug dbg, const Study &s);

};

#endif // study_h__