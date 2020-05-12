#ifndef SOMAFMSTATIONDELEGATE_H
#define SOMAFMSTATIONDELEGATE_H

#include <QStyledItemDelegate>

class SomaFMStationDelegate :
		public QStyledItemDelegate
{
public:
	SomaFMStationDelegate(QObject* parent=nullptr);
	~SomaFMStationDelegate() override;

public:
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif // SOMAFMSTATIONDELEGATE_H
