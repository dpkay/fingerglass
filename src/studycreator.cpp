#include "studycreator.h"

// tools
#include "fingerglasstool.h"
#include "directtouchtool.h"
#include "shifttool.h"
#include "dualfingerstretchtool.h"
#include "touchdisplaytool.h"
#include "precisionhandletool.h"

// tasks
#include "selectiontask.h"
#include "interactiontask.h"
#include "freestyletask.h"

// --------------------------------------------------------
Study * StudyCreator::createFromXML( const QString & file_name )
{
	QFile file(file_name);
	if(!file.open(QFile::ReadOnly | QFile::Text))
	{
		qFatal("cannot read xml file!");
	}

	// create document and read root node
	QDomDocument domDocument("fatfinger");
	if(!domDocument.setContent(&file))
	{
		qFatal("cannot parse xml file!");
	}
	QDomElement studyRootElement = domDocument.documentElement();
	if(studyRootElement.tagName().toLower() != "study")
	{
		qFatal("xml file has invalid root element!");
	}

	// create study and populate with tasks
	Study * study = new Study();
	QDomNodeList task_node_list = studyRootElement.childNodes();
	for(int i=0; i<task_node_list.size(); ++i)
	{
		if(task_node_list.item(i).nodeType() == QDomNode::ElementNode)
		{
			const QDomElement & task_element = task_node_list.item(i).toElement();
			if(task_element.nodeName().toLower() != "task")
			{
				qFatal("invalid task node found!");
			}
			Task * task = createTask(task_element);
			study->addTask(task);
		}
	}
	return study;
}

// --------------------------------------------------------
Task * StudyCreator::createTask( const QDomElement & root )
{
	// create task
	SettingContainer * task_setting_node = createSettingContainer(root);
	QString str_type = root.attributes().namedItem("type").toAttr().value();
	Task * task;
	if(str_type.toLower() == "selection")
	{
		task = new SelectionTask(task_setting_node);
	}
	else if(str_type.toLower() == "interaction")
	{
		task = new InteractionTask(task_setting_node);
	}
	else if(str_type.toLower() == "freestyle")
	{
		task = new FreestyleTask(task_setting_node);
	}
	else
	{
		qFatal("task node has invalid type!");
	}

	// read tools and add to task
	QDomNodeList tool_node_list = root.namedItem("tools").childNodes(); 
	for(int j=0; j<tool_node_list.size(); ++j)
	{
		if(tool_node_list.item(j).nodeType() == QDomNode::ElementNode)
		{
			const QDomElement & tool_element = tool_node_list.item(j).toElement();
			if(tool_element.nodeName().toLower() != "tool")
			{
				qFatal("invalid tool node found!");
			}

			Tool * tool = createTool(tool_element);
			task->addTool(tool);
		}
	}

	// read scene and add to task
	const QDomElement & scene_element = root.namedItem("scene").toElement();
	SettingContainer * scene_setting_node = createSettingContainer(scene_element);
	Scene * scene = createScene(scene_setting_node);
	task->setScene(scene);

	return task;
}

// --------------------------------------------------------
Scene * StudyCreator::createScene( SettingContainer * setting_container )
{
	return new Scene(setting_container);
}

// --------------------------------------------------------
Tool * StudyCreator::createTool( const QDomElement & root )
{
	QString str_type = root.attributes().namedItem("type").toAttr().value();
	SettingContainer * setting_container = createSettingContainer(root);
	Tool * result;

	if(str_type.toLower() == "fingerglass")
	{
		result = new FingerGlassTool(setting_container);
	}
	else if(str_type.toLower() == "directtouch")
	{
		result = new DirectTouchTool(setting_container);
	}
	else if(str_type.toLower() == "shift")
	{
		result = new ShiftTool(setting_container);
	}
	else if(str_type.toLower() == "dualfingerstretch")
	{
		result = new DualFingerStretchTool(setting_container);
	}
	else if(str_type.toLower() == "touchdisplay")
	{
		result = new TouchDisplayTool(setting_container);
	}
	else if(str_type.toLower() == "precisionhandle")
	{
		result = new PrecisionHandleTool(setting_container);
	}
	else
	{
		qFatal("tool node has invalid type!");
		exit(1);
	}
	return result;
}

// --------------------------------------------------------
SettingContainer * StudyCreator::createSettingContainer( const QDomElement & root )
{
	SettingContainer * result = new SettingContainer;
	QDomNodeList property_node_list = root.childNodes();
	for(int i=0; i<property_node_list.size(); ++i)
	{
		const QDomNode & property_node_candidate = property_node_list.item(i);
		if(property_node_candidate.nodeName().toLower() == "property")
		{
			QString str_name = property_node_candidate.attributes().namedItem("name").toAttr().value();
			QString str_type = property_node_candidate.attributes().namedItem("type").toAttr().value();
			QString str_value = property_node_candidate.attributes().namedItem("value").toAttr().value();
			if(str_type.toLower() == "string")
			{
				result->addStringProperty(str_name, str_value);
			}
			else if(str_type.toLower() == "int")
			{
				result->addIntProperty(str_name, str_value.toInt());
			}
			else if(str_type.toLower() == "real")
			{
				result->addFloatProperty(str_name, str_value.toFloat());
			}
			else if(str_type.toLower() == "bool")
			{
				bool value = true;
				if(str_value.toLower() == "" || str_value.toLower() == "0" || str_value.toLower() == "false")
				{
					value = false;
				}
				result->addBoolProperty(str_name, value);
			}
			else
			{
				qFatal("node property has invalid type!");
			}
		}
	}

	return result;
}