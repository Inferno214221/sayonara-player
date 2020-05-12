#include "SomaFMStationDelegate.h"
#include <QPainter>

SomaFMStationDelegate::SomaFMStationDelegate(QObject* parent) :
	QStyledItemDelegate(parent)
{}

SomaFMStationDelegate::~SomaFMStationDelegate() = default;

void SomaFMStationDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(index.column() != 0)
	{
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}
	painter->save();

	QPixmap pixmap = index.data(Qt::DecorationRole).value<QPixmap>();

	QRect r2 = option.rect;
	r2.setWidth((option.rect.width() * 30) / 40);
	r2.setHeight((option.rect.height() * 30) / 40);
	r2.translate((option.rect.bottomRight() - r2.bottomRight()) / 2);


	int minimum = std::min(r2.width(), r2.height());
	r2.setWidth(minimum);
	r2.setHeight(minimum);

	//painter->translate(option.rect.width() - r2.width() / 2, option.rect.height() - r2.height() / 2);
	painter->drawPixmap(r2, pixmap);
	painter->restore();
}
