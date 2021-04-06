/* HeaderView.cpp */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "HeaderView.h"

#include "Gui/Utils/Style.h"

#include <QAbstractItemModel>
#include <QColor>
#include <QFontMetrics>
#include <QPainter>
#include <QRect>
#include <QTextOption>

namespace
{
	void setFontBold(QPainter* painter)
	{
		auto font = painter->font();
		font.setBold(true);
		painter->setFont(font);
	}

	QString getText(QAbstractItemModel* model, int column, Qt::Orientation orientation)
	{
		return (model != nullptr)
		       ? model->headerData(column, orientation).toString()
		       : QString();
	}

	void drawText(QPainter* painter, const QRect& rect, const QString& text)
	{
		const auto fontMetrics = painter->fontMetrics();
		const auto elidedText = fontMetrics.elidedText(text, Qt::ElideRight, rect.width() - 5);
		painter->drawText(rect, elidedText, QTextOption(Qt::AlignVCenter | Qt::AlignHCenter));
	}

	void drawBorder(QPainter* painter, const QRect& rect)
	{
		const auto lineColor = QColor(36, 36, 36); // #242424
		painter->setPen(lineColor);
		painter->drawLine(rect.topRight(), rect.bottomRight());
	}

	void drawSortIndicator(QPainter* painter, const QRect& rect, bool ascending)
	{
		const auto arrow = ascending
			? QStringLiteral(u"▲")
			: QStringLiteral(u"▼");

		auto targetRect = rect;
		targetRect.setWidth(rect.width() - 4);
		painter->drawText(targetRect, arrow, QTextOption(Qt::AlignRight | Qt::AlignVCenter));
	}
}

void Gui::HeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
	if(Style::isDark())
	{
		painter->save();
		painter->fillRect(rect, painter->brush());

		const auto text = getText(model(), logicalIndex, this->orientation());
		setFontBold(painter);
		drawText(painter, rect, text);

		if(logicalIndex == this->sortIndicatorSection())
		{
			drawSortIndicator(painter, rect, (sortIndicatorOrder() == Qt::SortOrder::AscendingOrder));
		}

		drawBorder(painter, rect);

		painter->restore();
	}

	else
	{
		QHeaderView::paintSection(painter, rect, logicalIndex);
	}
}

