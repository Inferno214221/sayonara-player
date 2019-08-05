#include "LibraryPluginComboBoxDelegate.h"
#include <QPainter>
#include <QPalette>
#include <QColor>

LibraryPluginComboBoxDelegate::LibraryPluginComboBoxDelegate(QWidget* parent) :
	Gui::ComboBoxDelegate(parent),
	mParent(parent)
{}

LibraryPluginComboBoxDelegate::~LibraryPluginComboBoxDelegate() {}

#include <QFrame>
void LibraryPluginComboBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(index.data(Qt::AccessibleDescriptionRole).toString() == QLatin1String("separator") && index.row() == 1)
	{

		QColor color = painter->brush().color().dark();
		color.setAlpha(80);
		QPen pen(color);
		painter->setPen(pen);

		painter->drawLine(option.rect.left() + 2,
						  option.rect.center().y(),
						  option.rect.right() - 2,
						  option.rect.center().y());
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
		return QSize(0, 5);
	}

	return Gui::ComboBoxDelegate::sizeHint( option, index );
}
