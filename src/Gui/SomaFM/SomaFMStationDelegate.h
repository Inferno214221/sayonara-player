#ifndef SOMAFMSTATIONDELEGATE_H
#define SOMAFMSTATIONDELEGATE_H

#include "Gui/Utils/Delegates/StyledItemDelegate.h"

class SomaFMStationDelegate :
		public Gui::StyledItemDelegate
{
public:
	SomaFMStationDelegate(QObject* parent=nullptr);
	~SomaFMStationDelegate() override;
/*
public:
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;*/
};

#endif // SOMAFMSTATIONDELEGATE_H
