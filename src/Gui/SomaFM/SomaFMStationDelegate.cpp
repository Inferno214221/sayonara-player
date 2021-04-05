#include "SomaFMStationDelegate.h"

#include "Utils/Logger/Logger.h"
#include <QPainter>
#include <QPixmap>
#include <QIcon>

SomaFMStationDelegate::SomaFMStationDelegate(QObject* parent) :
	Gui::StyledItemDelegate(parent)
{}

SomaFMStationDelegate::~SomaFMStationDelegate() = default;
/*
void SomaFMStationDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(index.column() != 0)
	{
		Gui::StyledItemDelegate::paint(painter, option, index);
		return;
	}
	painter->save();

	const auto icon = index.data(Qt::DecorationRole).value<QIcon>();
	const auto pixmap = icon.pixmap(option.rect.size());

	if(pixmap.isNull())
	{
		spLog(Log::Info, this) << "Pixmap is NULL!";
	}

	painter->drawPixmap(option.rect, icon.pixmap(option.rect.size()));
	painter->restore();
}
*/