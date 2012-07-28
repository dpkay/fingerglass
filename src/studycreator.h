#ifndef studycreator_h__
#define studycreator_h__

#include "study.h"
#include "task.h"
#include "tool.h"
#include "settingcontainer.h"

#include <QDomElement>

class StudyCreator
{
public:
	Study * createFromXML(const QString & file_name);

protected:
	Task * createTask(const QDomElement & root);
	Scene * createScene(SettingContainer * setting_container);
	Tool * createTool(const QDomElement & root);
	SettingContainer * createSettingContainer(const QDomElement & root);

};

#endif // studycreator_h__
