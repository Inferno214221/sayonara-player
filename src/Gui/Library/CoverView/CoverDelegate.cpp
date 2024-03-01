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

		return {left, top, width, height};
	}

	void drawItemRectangle(QPainter* painter, const QStyleOptionViewItem& option, const int pixmapHeight)
	{
		constexpr const auto RectangleAlpha = 128;

		if(option.state & QStyle::State_Selected)
		{
			const auto rect = option.rect.translated(-option.rect.topLeft());
			painter->fillRect(rect, option.palette.highlight());
		}

		const auto rect = calcBoundingRect(option.rect, pixmapHeight);

		const auto oldColor = painter->pen().color();
		auto color = option.palette.highlightedText().color();
		color.setAlpha(RectangleAlpha); // NOLINT(*-magic-numbers)

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

	void paintTopOverlay(QPainter* painter, const QPalette& palette, const QString& text, const QRect& boundingRect)
	{
		if(text.isEmpty()) // NOLINT(*-magic-numbers)
		{
			return;
		}

		constexpr const auto RectAlpha = 192;

		auto color = palette.brush(QPalette::ColorGroup::Normal, QPalette::ColorRole::Window).color();
		color.setAlpha(RectAlpha);

		painter->fillRect(boundingRect, color);
		painter->drawText(boundingRect, text, QTextOption(Qt::AlignCenter | Qt::AlignHCenter));
	}

	constexpr const auto TopOverlayTextMargin = 4;
	constexpr const auto TopOverlayDistance = 3;

	QRect calcOverlayBoundingRect(const QPoint& offset, const QFontMetrics& fontMetrics, const QString& text)
	{
		return {offset, QPoint {fontMetrics.horizontalAdvance(text) + (4 * TopOverlayTextMargin),
		                        fontMetrics.height()
		}};
	}

	void paintRightTopOverlay(QPainter* painter, const QStyleOptionViewItem& option, const QString& text,
	                          const int verticalOffset)
	{
		auto boundingRect = calcOverlayBoundingRect({0, verticalOffset},
		                                            option.fontMetrics, text);
		boundingRect.translate(option.rect.width() - boundingRect.width() - TopOverlayDistance, 0);
		paintTopOverlay(painter, option.palette, text, boundingRect);
	}

	void paintLeftTopOverlay(QPainter* painter, const QStyleOptionViewItem& option, const QString& text,
	                         const int verticalOffset)
	{
		auto boundingRect = calcOverlayBoundingRect({TopOverlayDistance, verticalOffset}, option.fontMetrics, text);
		paintTopOverlay(painter, option.palette, text, boundingRect);
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

	constexpr const auto AlbumColor = 255;
	constexpr const auto ArtistColor = 172;

	const auto pixmapHeight = GetSetting(Set::Lib_CoverZoom);
	const auto verticalOffset = pixmapHeight / 20;

	painter->save();
	painter->translate(option.rect.left(), option.rect.top());

	drawItemRectangle(painter, option, pixmapHeight);

	const auto pixmap = index.data(CoverModel::CoverRole).value<QPixmap>();
	painter->translate(0, verticalOffset);
	paintPixmap(painter, option, pixmap, pixmapHeight);

	if(GetSetting(Set::Lib_CoverShowYear))
	{
		const auto year = index.data(CoverModel::YearRole).toInt();
		if(year > 1000)
		{
			paintLeftTopOverlay(painter, option, QString::number(year), -verticalOffset);
		}
	}

	painter->translate(0, pixmapHeight + 4);

	const auto album = index.data(CoverModel::AlbumRole).toString();
	drawText(painter, option, album, AlbumColor, GetSetting(Set::Lib_FontBold));
	painter->translate(0, option.fontMetrics.height());

	const auto artist = index.data(CoverModel::ArtistRole).toString();
	drawText(painter, option, artist, ArtistColor, false);

	painter->restore();
}
