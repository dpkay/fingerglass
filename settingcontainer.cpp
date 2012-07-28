#include "settingcontainer.h"

// --------------------------------------------------------
QDebug operator<<(QDebug dbg, const SettingContainer &node)
{
	dbg.nospace() << "\t\t\tSettingNode " << &node << " has the following " << node.size() << " properties:\n";
	foreach(const QString & key, node._string_properties.keys())
	{
		dbg.nospace() << "\t\t\t\t" << key << ": " << node._string_properties.value(key) << "\n";
	}
	foreach(const QString & key, node._int_properties.keys())
	{
		dbg.nospace() << "\t\t\t\t" << key << ": " << node._int_properties.value(key) << "\n";
	}
	foreach(const QString & key, node._float_properties.keys())
	{
		dbg.nospace() << "\t\t\t\t" << key << ": " << node._float_properties.value(key) << "\n";
	}
	foreach(const QString & key, node._bool_properties.keys())
	{
		dbg.nospace() << "\t\t\t\t" << key << ": " << node._bool_properties.value(key) << "\n";
	}
	return dbg.space();
}