#include "LibraryPluginComboBoxDelegate.h"
#include <QPainter>
#include <QPalette>
#include <QColor>

LibraryPluginComboBoxDelegate::LibraryPluginComboBoxDelegate(QObject* parent) : Gui::ComboBoxDelegate(parent) {}

LibraryPluginComboBoxDelegate::~LibraryPluginComboBoxDelegate() {}

void LibraryPluginComboBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(index.data(Qt::AccessibleDescriptionRole).toString() == QLatin1String("separator"))
	{
		QColor color = option.palette.color(QPalette::Disabled, QPalette::Foreground);
		color.setAlpha(196);
		painter->setPen(color);

		//painter->translate(4, 0);
		painter->setBrush(option.palette.color(QPalette::Active, QPalette::Background));
		painter->fillRect(option.rect, painter->brush());
		painter->drawLine(option.rect.left() + 4, option.rect.center().y(), option.rect.right() - 4, option.rect.center().y());
	}

	else {
		Gui::ComboBoxDelegate::paint(painter, option, index);
	}
}

QSize LibraryPluginComboBoxDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QString type = index.data(Qt::AccessibleDescriptionRole).toString();

	if(type == QLatin1String("separator"))
	{
		return QSize(0, 1);
	}

	return Gui::ComboBoxDelegate::sizeHint( option, index );
}
