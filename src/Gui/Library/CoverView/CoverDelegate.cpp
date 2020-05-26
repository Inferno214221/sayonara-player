/* AlbumCoverDelegate.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "CoverModel.h"                    // Role names
#include "CoverDelegate.h"
#include "Utils/Settings/Settings.h"

#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QFontMetrics>

Library::CoverDelegate::CoverDelegate(QObject* parent) :
	QStyledItemDelegate(parent) {}

Library::CoverDelegate::~CoverDelegate() = default;

void
Library::CoverDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	const int textOffset = 3;

	QFontMetrics fm = option.fontMetrics;
	int zoom = GetSetting(Set::Lib_CoverZoom);

	painter->save();
	painter->translate(option.rect.x(), option.rect.y());

	{
		const QPixmap pm = index.data(CoverModel::CoverRole).value<QPixmap>();
		if(pm.isNull())
		{
			painter->restore();
			return;
		}

		painter->translate(0, zoom / 20);

		int xZoom = (option.rect.width() - zoom) / 2;

		QPen pen = painter->pen();
		QColor oldColor = pen.color();

		QColor color = option.palette.color(QPalette::Active, QPalette::Highlight);
		pen.setColor(color);
		painter->setPen(pen);

		painter->drawRect(xZoom - 2, -2, zoom + 3, zoom + 3);

		pen.setColor(oldColor);
		painter->setPen(pen);

		painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
		QRectF target(xZoom, 0, zoom, zoom);
		QRectF source(0, 0, pm.width(), pm.height());
		painter->drawPixmap(target, pm, source);

		painter->translate(0, zoom + 4);
	}

	{
		QString album = index.data(CoverModel::AlbumRole).toString();
		album = fm.elidedText(album, Qt::ElideRight, option.rect.width() - 2 * textOffset);
		painter->drawText(textOffset,
		                  0,
		                  option.rect.width() - 2 * textOffset,
		                  fm.height(),
		                  int(option.displayAlignment),
		                  album);
		painter->translate(0, fm.height());
	}

	{
		QString artist = index.data(CoverModel::ArtistRole).toString();
		artist = fm.elidedText(artist, Qt::ElideRight, option.rect.width() - 2 * textOffset);
		if(!artist.isEmpty())
		{
			QPen pen = painter->pen();
			QColor color = pen.color();
			color.setAlpha(172);
			pen.setColor(color);
			painter->setPen(pen);
			painter->drawText(textOffset,
			                  0,
			                  option.rect.width() - 2 * textOffset,
			                  fm.height(),
			                  option.displayAlignment,
			                  artist);
		}
	}

	painter->restore();
}
