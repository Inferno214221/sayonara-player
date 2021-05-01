/* PlaylistItemDelegate.cpp */

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

#include "PlaylistModel.h"
#include "PlaylistDelegate.h"

#include "Utils/Settings/Settings.h"

#include "Gui/Utils/Widgets/RatingLabel.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Icons.h"

#include <QPainter>
#include <QIcon>

using Gui::RatingEditor;
using Gui::RatingLabel;
using Playlist::Delegate;
using Playlist::Model;

namespace
{
	struct PlaylistStyleItem
	{
		QString text;
		bool isBold {false};
		bool isItalic {false};
	};

	QList<PlaylistStyleItem> parseEntryLookString(const QModelIndex& index)
	{
		const auto entryLook = index.data(Model::EntryLookRole).toString();

		QList<PlaylistStyleItem> ret;
		PlaylistStyleItem currentItem;

		for(const auto c : entryLook)
		{
			if((c != QChar(Model::StyleElement::Bold)) && (c != QChar(Model::StyleElement::Italic)))
			{
				currentItem.text += c;
				continue;
			}

			ret << currentItem;
			currentItem.text.clear();

			if(c == QChar(Model::StyleElement::Bold))
			{
				currentItem.isBold = !currentItem.isBold;
			}

			else if(c == QChar(Model::StyleElement::Italic))
			{
				currentItem.isItalic = !currentItem.isItalic;
			}
		}

		if(!currentItem.text.isEmpty())
		{
			ret << currentItem;
		}

		return ret;
	}

	inline bool isTrackNumberColumn(const QModelIndex& index)
	{
		return (index.column() == Model::ColumnName::TrackNumber);
	}

	inline bool isCurrentTrack(const QModelIndex& index)
	{
		return index.data(Model::CurrentPlayingRole).toBool();
	}

	inline bool isDragIndex(const QModelIndex& index)
	{
		return index.data(Model::DragIndexRole).toBool();
	}

	inline Rating parseRating(const QModelIndex& index)
	{
		return index.data(Model::RatingRole).value<Rating>();
	}

	void drawDragDropLine(QPainter* painter, const QRect& rect)
	{
		const auto y = rect.topLeft().y() + rect.height() - 1;
		painter->drawLine(QLine(rect.x(), y, rect.x() + rect.width(), y));
	}

	void setTextColor(QPainter* painter, const QStyleOptionViewItem& option)
	{
		const auto isSelected = (option.state & QStyle::State_Selected);
		const auto isEnabled = (option.state & QStyle::State_Enabled);

		auto textColor = (isSelected)
		                 ? option.palette.color(QPalette::Active, QPalette::HighlightedText)
		                 : option.palette.color(QPalette::Active, QPalette::WindowText);

		if(!isEnabled)
		{
			textColor.setAlpha(196);
		}

		auto pen = painter->pen();
		pen.setColor(textColor);
		painter->setPen(pen);
	}

	void setFontStyle(QPainter* painter, bool isBold, bool isItalic)
	{
		auto font = painter->font();
		font.setBold(isBold);
		font.setItalic(isItalic);
		painter->setFont(font);
	}

	void drawStyleItem(QPainter* painter, const PlaylistStyleItem& styleItem, bool alignTop, QRect& rect)
	{
		setFontStyle(painter, styleItem.isBold, styleItem.isItalic);

		const auto alignment = (alignTop)
		                       ? (Qt::AlignLeft | Qt::AlignTop)
		                       : (Qt::AlignLeft | Qt::AlignVCenter);

		const auto fontMetric = painter->fontMetrics();
		painter->drawText(rect, alignment, fontMetric.elidedText(styleItem.text, Qt::ElideRight, rect.width()));

		const auto horizontalOffset = Gui::Util::textWidth(fontMetric, styleItem.text);
		rect.setWidth(rect.width() - horizontalOffset);
		rect.translate(horizontalOffset, 0);
	}

	void drawTrackMetadata(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, bool alignTop)
	{
		setTextColor(painter, option);

		auto rect = (alignTop)
		            ? QRect(option.rect.left(), option.rect.y() + 1, option.rect.width(), option.rect.height() - 2)
		            : option.rect;

		const auto styleItems = parseEntryLookString(index);
		for(const auto& styleItem : styleItems)
		{
			drawStyleItem(painter, styleItem, alignTop, rect);
		}
	}

	void paintRatingLabel(QPainter* painter, const Rating& rating, const QRect& rect)
	{
		if(rating != Rating::Last)
		{
			painter->translate(0, 2);

			auto ratingLabel = RatingLabel(nullptr, true);
			ratingLabel.setRating(rating);
			ratingLabel.setVerticalOffset(rect.height() / 2);
			ratingLabel.paint(painter, rect);
		}
	}

	void paintPlayPixmap(QPainter* painter, const QRect& rect)
	{
		constexpr const auto ScaleFactor = 10;

		const auto icon = Gui::Icons::icon(Gui::Icons::Play);

		const auto yTop = rect.y() + (rect.height() / ScaleFactor);
		const auto height = (rect.height() * (ScaleFactor - 2)) / ScaleFactor;
		const auto xLeft = rect.x() + (rect.width() - height) / 2;

		painter->drawPixmap(xLeft, yTop, icon.pixmap(height, height).scaledToHeight(height));
	}
}

Delegate::Delegate(QObject* parent) :
	StyledItemDelegate(parent)
{}

Delegate::~Delegate() = default;

void Delegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(!index.isValid())
	{
		return;
	}

	StyledItemDelegate::paint(painter, option, index);

	if(isCurrentTrack(index) && isTrackNumberColumn(index))
	{
		paintPlayPixmap(painter, option.rect);
	}

	if(isDragIndex(index))
	{
		drawDragDropLine(painter, option.rect);
	}

	if(index.column() == Model::ColumnName::Description)
	{
		painter->save();

		const auto showRating = GetSetting(Set::PL_ShowRating);
		drawTrackMetadata(painter, option, index, showRating);

		if(showRating)
		{
			const auto rating = parseRating(index);
			paintRatingLabel(painter, rating, option.rect);
		}

		painter->restore();
	}
}

QWidget* Delegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	const auto rating = parseRating(index);
	if(rating != Rating::Last)
	{
		auto* ratingEditor = new RatingEditor(rating, parent);
		ratingEditor->setVerticalOffset(option.rect.height() / 2);

		connect(ratingEditor, &RatingEditor::sigFinished, this, &Delegate::deleteEditor);

		return ratingEditor;
	}

	return nullptr;
}

void Delegate::deleteEditor([[maybe_unused]] bool save)
{
	auto* ratingEditor = qobject_cast<RatingEditor*>(sender());
	if(ratingEditor)
	{
		disconnect(ratingEditor, &RatingEditor::sigFinished, this, &Delegate::deleteEditor);

		emit commitData(ratingEditor);
		emit closeEditor(ratingEditor);
	}
}

void Delegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto* ratingEditor = qobject_cast<RatingEditor*>(editor);
	if(!ratingEditor)
	{
		const auto rating = parseRating(index);
		ratingEditor->setRating(rating);
	}
}

void Delegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto* ratingEditor = qobject_cast<RatingEditor*>(editor);
	if(ratingEditor)
	{
		const auto rating = ratingEditor->rating();
		model->setData(index, QVariant::fromValue(rating));
	}
}
