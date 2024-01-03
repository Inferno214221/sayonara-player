/* AlbumCoverDelegate.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "CoverModel.h"
#include "CoverDelegate.h"
#include "Utils/Settings/Settings.h"

#include <QColor>
#include <QPainter>
#include <QFontMetrics>

namespace
{
	QRect calcBoundingRect(const QRect& optionRect, const int pixmapHeight)
	{
		const auto height = pixmapHeight + 2;
		const auto width = height;
		const auto left = (optionRect.width() - width) / 2;
		const auto top = (pixmapHeight / 20) - 1;

		auto rect = QRect(left, top, width, height);
		rect.translate(optionRect.topLeft());

		return rect;
	}

	void drawItemRectangle(QPainter* painter, const QStyleOptionViewItem& option, const int pixmapHeight)
	{
		if(option.state & QStyle::State_Selected)
		{
			painter->fillRect(option.rect, option.palette.highlight());
		}

		const auto rect = calcBoundingRect(option.rect, pixmapHeight);

		const auto oldColor = painter->pen().color();
		auto color = option.palette.highlightedText().color();
		color.setAlpha(128);

		painter->setPen(color);
		painter->drawRect(rect);
		painter->setPen(oldColor);
	}

	void paintPixmap(QPainter* painter, const QStyleOptionViewItem& option, const QPixmap& pixmap, int height)
	{
		if(!pixmap.isNull())
		{
			const auto left = (option.rect.width() - height) / 2;
			const auto target = QRect(left, 0, height, height);
			const auto source = QRect(0, 0, pixmap.width(), pixmap.height());

			painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
			painter->drawPixmap(target, pixmap, source);
		}
	}

	void drawText(QPainter* painter, const QStyleOptionViewItem& option, const QString& text, int alpha, bool bold)
	{
		constexpr const auto textOffset = 3;
		const auto textWidth = option.rect.width() - (2 * textOffset);

		const auto oldPen = painter->pen();
		auto textColor = (option.state & QStyle::State_Selected)
		                 ? option.palette.highlightedText().color()
		                 : option.palette.text().color();
		textColor.setAlpha(alpha);

		auto font = painter->font();
		font.setBold(bold);

		const auto fontMetrics = QFontMetrics(font);
		const auto elidedText = fontMetrics.elidedText(text, Qt::ElideRight, textWidth);

		painter->setFont(font);
		painter->setPen(textColor);
		painter->drawText(QRect {textOffset, 0, textWidth, fontMetrics.height()},
		                  static_cast<int>(option.displayAlignment),
		                  elidedText);

		painter->setPen(oldPen);
	}
}

void
Library::CoverDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(!index.isValid())
	{
		// may happen if not all columns are filled within the last row
		return;
	}

	painter->save();

	const auto pixmapHeight = GetSetting(Set::Lib_CoverZoom);
	drawItemRectangle(painter, option, pixmapHeight);
	painter->translate(option.rect.x(), option.rect.y());

	const auto pixmap = index.data(CoverModel::CoverRole).value<QPixmap>();
	painter->translate(0, pixmapHeight / 20);
	paintPixmap(painter, option, pixmap, pixmapHeight);
	painter->translate(0, pixmapHeight + 4);

	const auto album = index.data(CoverModel::AlbumRole).toString();
	drawText(painter, option, album, 255, GetSetting(Set::Lib_FontBold));
	painter->translate(0, option.fontMetrics.height());

	const auto artist = index.data(CoverModel::ArtistRole).toString();
	drawText(painter, option, artist, 172, false);

	painter->restore();
}
