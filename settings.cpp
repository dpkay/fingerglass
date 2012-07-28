#include "settings.h"
#include <cstdlib>

Settings * Settings::_instance = 0;

// --------------------------------------------------------
const Settings * Settings::instance()
{
	if(_instance == 0)
	{
		_instance = new Settings;
	}

	return _instance;
}

// --------------------------------------------------------
Settings::Settings()
{
	// load default settings
	//_mw_glass_count = 3;
	//_mw_ellipse_ratio = 3;
	//_mw_magnification_multiplier = 2;
	//_mw_resizable = false;
	//_mw_src_resizing_maintains_ratio = true;

	_mw_glass_count = 1;
	_mw_ellipse_ratio = 1;
	_mw_magnification_multiplier = 2;
	_mw_resizable = true;
	_mw_src_resizing_maintains_ratio = false;
	_mw_stabilized = true;
}