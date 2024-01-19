/* PlaylistItemDelegate.cpp */

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
	constexpr const auto RatingLabelOffsetX = 8;
	constexpr const auto RatingLabelOffsetY = 2;

	bool isEnabled(const QModelIndex& index)
	{
		return index.data(Model::Roles::EnabledRole).toBool();
	}

	inline bool isCurrentTrack(const QModelIndex& index)
	{
		return index.data(Model::CurrentPlayingRole).toBool();
	}

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

		for(const auto c: entryLook)
		{
			if((c != QChar(Model::StyleElement::Bold)) &&
			   (c != QChar(Model::StyleElement::Italic)))
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
		painter->drawLine(rect.x(), y, rect.x() + rect.width(), y);
	}

	QColor getCurrentTrackColor(bool hasCustomColor, const QString& customColor, const QColor& standardColor)
	{
		if(hasCustomColor)
		{
			if(const auto color = QColor(customColor); color.isValid())
			{
				return color;
			}
		}

		return standardColor;
	}

	QColor getTextColor(const QStyleOptionViewItem& option, bool isCurrentTrack)
	{
		const auto& standardColor = option.palette.color(QPalette::Active, QPalette::WindowText);
		if(isCurrentTrack)
		{
			if(Style::isDark())
			{
				return getCurrentTrackColor(GetSetting(Set::PL_CurrentTrackCustomColorDark),
				                            GetSetting(Set::PL_CurrentTrackColorStringDark),
				                            {standardColor});
			}

			return getCurrentTrackColor(GetSetting(Set::PL_CurrentTrackCustomColorStandard),
			                            GetSetting(Set::PL_CurrentTrackColorStringStandard),
			                            {standardColor});
		}

		return standardColor;
	}

	void setTextColor(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index)
	{
		const auto isSelected = (option.state & QStyle::State_Selected);
		const auto isEnabled = ::isEnabled(index);
		const auto isCurrentTrack = ::isCurrentTrack(index);

		auto textColor = (isSelected)
		                 ? option.palette.color(QPalette::Active, QPalette::HighlightedText)
		                 : getTextColor(option, isCurrentTrack);

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

		const auto alignment = alignTop
		                       ? static_cast<int>(Qt::AlignLeft | Qt::AlignTop)
		                       : static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);

		const auto fontMetric = painter->fontMetrics();
		painter->drawText(rect, alignment, fontMetric.elidedText(styleItem.text, Qt::ElideRight, rect.width()));

		const auto horizontalOffset = Gui::Util::textWidth(fontMetric, styleItem.text);
		rect.setWidth(rect.width() - horizontalOffset);
		rect.translate(horizontalOffset, 0);
	}

	void
	drawTrackMetadata(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, bool alignTop)
	{
		setTextColor(painter, option, index);

		auto rect = (alignTop)
		            ? QRect(option.rect.left(), option.rect.y() + 1, option.rect.width(), option.rect.height() - 2)
		            : option.rect;

		const auto styleItems = parseEntryLookString(index);
		for(const auto& styleItem: styleItems)
		{
			drawStyleItem(painter, styleItem, alignTop, rect);
		}
	}

	template<typename Widget>
	void paintRatingWidget(QPainter* painter, Widget* widget, const QRect& rect)
	{
		widget->setVerticalOffset(rect.height() / 2);
		widget->paint(painter, rect);
	}

	void paintRatingLabel(QPainter* painter, const Rating rating, const QRect& rect)
	{
		if(rating != Rating::Last)
		{
			painter->translate(0, RatingLabelOffsetY);

			auto ratingLabel = RatingLabel(nullptr, true);
			ratingLabel.setRating(rating);
			paintRatingWidget(painter, &ratingLabel, rect);
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
	StyledItemDelegate(parent) {}

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
		painter->translate(RatingLabelOffsetX, 0);

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

QWidget* Delegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
	if(const auto rating = parseRating(index); (rating != Rating::Last))
	{
		auto* ratingEditor = new RatingEditor(rating, parent);

		connect(ratingEditor, &RatingEditor::sigFinished, this, &Delegate::deleteEditor);

		return ratingEditor;
	}

	return nullptr;
}

void Delegate::deleteEditor([[maybe_unused]] bool save)
{
	if(auto* ratingEditor = dynamic_cast<RatingEditor*>(sender()); ratingEditor)
	{
		disconnect(ratingEditor, &RatingEditor::sigFinished, this, &Delegate::deleteEditor);

		emit commitData(ratingEditor);
		emit closeEditor(ratingEditor);
	}
}

void Delegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	if(auto* ratingEditor = dynamic_cast<RatingEditor*>(editor); ratingEditor)
	{
		const auto rating = parseRating(index);
		ratingEditor->setRating(rating);
	}
}

void Delegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	if(auto* ratingEditor = dynamic_cast<RatingEditor*>(editor); ratingEditor)
	{
		const auto rating = ratingEditor->rating();
		model->setData(index, QVariant::fromValue(rating));
	}
}

void Playlist::Delegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                              const QModelIndex& /*index*/) const
{
	if(auto* ratingEditor = dynamic_cast<RatingEditor*>(editor); ratingEditor)
	{
		ratingEditor->setVerticalOffset(option.rect.height() / 2);

		auto rect = option.rect;
		rect.translate(RatingLabelOffsetX, RatingLabelOffsetY);
		ratingEditor->setGeometry(rect);
	}
}
