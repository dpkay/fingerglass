#ifndef touchwidget_h__
#define touchwidget_h__

#include "touchwidgetcontroller.h"
#include "touchwidgetrenderer.h"

class TouchWidget
{
public:
	TouchWidget(
		//TouchWidgetType type,
		TouchWidgetController * controller,
		TouchWidgetRenderer * renderer
	);

	TouchWidgetType type();
	TouchWidgetController * controller();
	TouchWidgetRenderer * renderer();
	const TouchWidgetController * controller() const;
	const TouchWidgetRenderer * renderer() const;

private:
	//TouchWidgetType _type;
	TouchWidgetController * _controller;
	TouchWidgetRenderer * _renderer;
};

#endif // touchwidget_h__