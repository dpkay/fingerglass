#ifndef settingnode_h__
#define settingnode_h__

#include <QMap>

class SettingContainer
{
public:
	QString stringProperty(const QString & name) const { Q_ASSERT(_string_properties.contains(name)); return _string_properties.value(name); }
	int intProperty(const QString & name) const { Q_ASSERT(_int_properties.contains(name)); return _int_properties.value(name); }
	float floatProperty(const QString & name) const { Q_ASSERT(_float_properties.contains(name)); return _float_properties.value(name); }
	bool boolProperty(const QString & name) const { Q_ASSERT(_bool_properties.contains(name)); return _bool_properties.value(name); }
	void addStringProperty(const QString & name, const QString & value)
	{
		Q_ASSERT(!_string_properties.contains(name));
		_string_properties[name] = value;
	}
	void addIntProperty(const QString & name, int value)
	{
		Q_ASSERT(!_int_properties.contains(name));
		_int_properties[name] = value;
	}
	void addFloatProperty(const QString & name, float value)
	{
		Q_ASSERT(!_float_properties.contains(name));
		_float_properties[name] = value;
	}
	void addBoolProperty(const QString & name, bool value)
	{
		Q_ASSERT(!_bool_properties.contains(name));
		_bool_properties[name] = value;
	}
	bool hasStringProperty(const QString & name) { return this->_string_properties.contains(name); }
	bool hasIntProperty(const QString & name) { return this->_int_properties.contains(name); }
	bool hasFloatProperty(const QString & name) { return this->_float_properties.contains(name); }
	bool hasBoolProperty(const QString & name) { return this->_bool_properties.contains(name); }
	int size() const
	{
		return _string_properties.size() + _int_properties.size() + _float_properties.size() + _bool_properties.size();
	}

	void serialize( QDataStream * out )
	{	
		*out << _string_properties;
		*out << _int_properties;
		*out << _float_properties;
		*out << _bool_properties;
	}

private:
	QMap<QString, QString> _string_properties;
	QMap<QString, int> _int_properties;
	QMap<QString, float> _float_properties;
	QMap<QString, bool> _bool_properties;

	friend QDebug operator<<(QDebug dbg, const SettingContainer &node);
};

#endif // settingnode_h__