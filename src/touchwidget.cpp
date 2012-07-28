#include "touchwidget.h"

// --------------------------------------------------------
TouchWidget::TouchWidget(
	TouchWidgetController * controller,
	TouchWidgetRenderer * renderer
)
{
	Q_ASSERT(controller != NULL);
	Q_ASSERT(renderer != NULL);
	this->_controller = controller;
	this->_renderer = renderer;
	//this->_type = type;
}

// --------------------------------------------------------
TouchWidgetController * TouchWidget::controller()
{
	return this->_controller;
}

// --------------------------------------------------------
const TouchWidgetController * TouchWidget::controller() const
{
	return this->_controller;
}
// --------------------------------------------------------
TouchWidgetRenderer * TouchWidget::renderer()
{
	return this->_renderer;
}

// --------------------------------------------------------
const TouchWidgetRenderer * TouchWidget::renderer() const
{
	return this->_renderer;
}

// --------------------------------------------------------
TouchWidgetType TouchWidget::type()
{
	//return _type;
	return controller()->type();
}