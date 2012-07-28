#ifndef settings_h__
#define settings_h__

class Settings
{
public:
	Settings();

	int mwGlassCount() const { return _mw_glass_count; }
	float mwEllipseRatio() const { return _mw_ellipse_ratio; }
	float mwMagnificationMultiplier() const { return _mw_magnification_multiplier; }
	bool mwResizable() const { return _mw_resizable; }
	bool mwSrcResizingMaintainsRatio() const { return _mw_src_resizing_maintains_ratio; }
	bool mwStabilized() const { return _mw_stabilized; }

	static const Settings * instance();

private:
	int _mw_glass_count;
	float _mw_ellipse_ratio;
	float _mw_magnification_multiplier;
	bool _mw_resizable;
	bool _mw_src_resizing_maintains_ratio;
	bool _mw_stabilized;

	static Settings * _instance;

};

#endif // settings_h__