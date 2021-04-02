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
#include "Utils/MetaData/RadioMode.h"

#include "Gui/Utils/Widgets/RatingLabel.h"
#include "Gui/Utils/Widgets/SpectrumLabel.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/GuiUtils.h"

#include <QPainter>
#include <QFontMetrics>
#include <QTableView>

const static int PLAYLIST_BOLD = 70;
using Gui::RatingEditor;
using Gui::RatingLabel;
using Playlist::Delegate;
using Playlist::Model;

struct PlaylistStyleItem
{
	QString text;

	bool isBold;
	bool isItalic;

	PlaylistStyleItem() :
		isBold(false), isItalic(false) {}
};

static QList<PlaylistStyleItem> parseEntryLookString(const QString& entryLook)
{
	QList<PlaylistStyleItem> ret;
	PlaylistStyleItem currentItem;

	for(QChar c : entryLook)
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

struct Delegate::Private
{
	SpectrumLabel* spectrum = nullptr;

	int ratingHeight;
	bool showRating;

	Private() :
		ratingHeight(18),
		showRating(false)
	{
		showRating = GetSetting(Set::PL_ShowRating);
	}
};

Delegate::Delegate(QTableView* parent) :
	StyledItemDelegate(parent)
{
	m = Pimpl::make<Private>();

	//m->spectrum = new SpectrumLabel(nullptr);

	ListenSettingNoCall(Set::PL_ShowRating, Delegate::playlistShowRatingChanged);
}

Delegate::~Delegate() = default;

void Delegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(!index.isValid())
	{
		return;
	}

	QPalette palette = option.palette;
	QRect rect(option.rect);

	StyledItemDelegate::paint(painter, option, index);

	// drag and drop active
	if(index.data(Model::DragIndexRole).toBool() == true)
	{
		int y = rect.topLeft().y() + rect.height() - 1;
		painter->drawLine(QLine(rect.x(), y, rect.x() + rect.width(), y));
	}

	// finished if not the middle column
	if(index.column() != Model::ColumnName::Description)
	{
		return;
	}

	painter->save();

	{ // give that pen some alpha value, so it appears lighter
		bool isEnabled = (option.state & QStyle::State_Enabled);
		if(!isEnabled)
		{
			QColor textColor = palette.color(QPalette::Disabled, QPalette::WindowText);
			if(Style::isDark())
			{
				textColor.setAlpha(196);
			}

			QPen pen = painter->pen();
			pen.setColor(textColor);
			painter->setPen(pen);
		}
	}

	QFont font = option.font;
	{ // set the font
		/*font.setPointSizeF(GetSetting(Set::Player_ScalingFactor) * font.pointSizeF());
		painter->setFont(font);*/
	}

	int alignment = int(Qt::AlignLeft);
	{ // set alignment
		if(m->showRating)
		{
			alignment |= Qt::AlignTop;
			rect.setY(option.rect.y() + 2);
		}

		else
		{
			alignment |= Qt::AlignVCenter;
		}
	}

	int xOffset = 4;
	int standardWeight = font.weight();

	const QString entryLook = index.data(Model::EntryLookRole).toString();
	QList<PlaylistStyleItem> styleItems = parseEntryLookString(entryLook);
	for(const PlaylistStyleItem& item : styleItems)
	{
		font.setWeight(item.isBold ? PLAYLIST_BOLD : standardWeight);
		font.setItalic(item.isItalic);
		painter->setFont(font);

		QFontMetrics fm(font);
		painter->translate(xOffset, 0);
		painter->drawText(rect, alignment, fm.elidedText(item.text, Qt::ElideRight, rect.width()));

		xOffset = Gui::Util::textWidth(fm, item.text);
		rect.setWidth(rect.width() - xOffset);
	}

	if(m->showRating)
	{
		painter->restore();
		painter->save();

		RadioMode radioMode = RadioMode(index.data(Model::RadioModeRole).toInt());
		if(radioMode != RadioMode::Station)
		{
			painter->translate(0, 2);

			Rating rating = index.data(Model::RatingRole).value<Rating>();

			RatingLabel ratingLabel(nullptr, true);
			ratingLabel.setRating(rating);
			ratingLabel.setVerticalOffset(option.rect.height() - m->ratingHeight);
			ratingLabel.paint(painter, option.rect);
		}
	}

	painter->restore();
}

void Delegate::playlistShowRatingChanged()
{
	m->showRating = GetSetting(Set::PL_ShowRating);
}

QWidget* Delegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option)

	Rating rating = index.data(Qt::EditRole).value<Rating>();
	if(rating == Rating::Last)
	{
		return nullptr;
	}

	auto* editor = new RatingEditor(rating, parent);
	editor->setVerticalOffset(option.rect.height() - m->ratingHeight);

	connect(editor, &RatingEditor::sigFinished, this, &Delegate::deleteEditor);

	return editor;
}

void Delegate::deleteEditor(bool save)
{
	Q_UNUSED(save)

	auto* editor = qobject_cast<RatingEditor*>(sender());
	if(!editor)
	{
		return;
	}

	disconnect(editor, &RatingEditor::sigFinished, this, &Delegate::deleteEditor);

	emit commitData(editor);
	emit closeEditor(editor);
}

void Delegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto* rating_editor = qobject_cast<RatingEditor*>(editor);
	if(!rating_editor)
	{
		return;
	}

	Rating rating = index.data(Qt::EditRole).value<Rating>();
	rating_editor->setRating(rating);
}

void Delegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto* rating_editor = qobject_cast<RatingEditor*>(editor);
	if(rating_editor)
	{
		Rating rating = rating_editor->rating();
		model->setData(index, QVariant::fromValue(rating));
	}
}
