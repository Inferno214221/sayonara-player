#include "SomaFMStationDelegate.h"
#include <QPainter>

SomaFMStationDelegate::SomaFMStationDelegate(QObject* parent) :
	Gui::StyledItemDelegate(parent)
{}

SomaFMStationDelegate::~SomaFMStationDelegate() = default;

void SomaFMStationDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(index.column() != 0)
	{
		Gui::StyledItemDelegate::paint(painter, option, index);
		return;
	}
	painter->save();

	QPixmap pixmap = index.data(Qt::DecorationRole).value<QPixmap>();

	const QList<int> rounds{24, 32, 36, 48};

	QRect r2 = option.rect;
	r2.setWidth((option.rect.width() * 30) / 40);
	r2.setHeight((option.rect.height() * 30) / 40);
	int minimum = std::min(r2.width(), r2.height());

	auto it = std::min_element(rounds.begin(), rounds.end(), [minimum](int r1, int r2){
		return (std::abs(minimum - r1) < std::abs(minimum - r2));
	});

	r2.setWidth(*it);
	r2.setHeight(*it);
	r2.translate((option.rect.bottomRight() - r2.bottomRight()) / 2);

	painter->drawPixmap(r2, pixmap);
	painter->restore();
}
