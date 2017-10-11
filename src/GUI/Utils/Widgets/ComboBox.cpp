#include "ComboBox.h"
#include "GUI/Utils/Delegates/ComboBoxDelegate.h"
#include "GUI/Utils/Delegates/StyledItemDelegate.h"
#include "Utils/Settings/Settings.h"

#include <QAbstractItemView>
#include <QEvent>

using Gui::ComboBox;
using Gui::WidgetTemplate;

ComboBox::ComboBox(QWidget* parent) :
	WidgetTemplate<QComboBox>(nullptr)
{
	Q_UNUSED(parent)

	this->setItemDelegate(new ComboBoxDelegate(this));
}

ComboBox::~ComboBox() {}

void ComboBox::changeEvent(QEvent* event)
{
	WidgetTemplate<QComboBox>::changeEvent(event);

	if(event->type() != QEvent::StyleChange){
		return;
	}

	QFontMetrics f(this->font());
	int h = f.height();
	h = std::max(h, 16);

	this->setIconSize(QSize(h, h));
	update();
	repaint();
}

