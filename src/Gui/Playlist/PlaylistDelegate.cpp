/* PlaylistItemDelegate.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "Utils/MetaData/MetaData.h"
#include "Utils/Settings/Settings.h"

#include "Gui/Utils/Widgets/RatingLabel.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/GuiUtils.h"

#include <QPainter>
#include <QFontMetrics>
#include <QTableView>
#include <array>

const static int PLAYLIST_BOLD=70;
using Gui::RatingEditor;
using Gui::RatingLabel;
using Playlist::Delegate;

struct Delegate::Private
{
	QString		entry_look;
	int			rating_height;
	bool		show_rating;

	Private() :
		rating_height(18),
		show_rating(false)
	{
		entry_look = GetSetting(Set::PL_EntryLook);
		show_rating = GetSetting(Set::PL_ShowRating);
	}
};

Delegate::Delegate(QTableView* parent) :
	StyledItemDelegate(parent)
{
	m = Pimpl::make<Private>();

	ListenSettingNoCall(Set::PL_EntryLook, Delegate::sl_look_changed);
	ListenSettingNoCall(Set::PL_ShowRating, Delegate::sl_show_rating_changed);
}

Delegate::~Delegate() = default;

void Delegate::paint(QPainter *painter,	const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if(!index.isValid()) {
		return;
	}

	QPalette palette = option.palette;
	QRect rect(option.rect);

	int col = index.column();
	int row = index.row();
	int row_height = rect.height();

	StyledItemDelegate::paint(painter, option, index);

	{ // drag and drop active
		if(index.data(Qt::UserRole).toBool() == true)
		{
			int y = rect.topLeft().y() + row_height - 1;
			painter->drawLine(QLine(rect.x(), y, rect.x() + rect.width(), y));
		}
	}

	{ // finished if not the middle column
		if(col != Model::ColumnName::Description) {
			return;
		}
	}

	auto* model = static_cast<const Model*>(index.model());
	const MetaData& md = model->metadata(row);

	painter->save();
	{ // give that pen some alpha value, so it appears lighter
		if(md.is_disabled())
		{
			QColor col_text = palette.color(QPalette::Disabled, QPalette::WindowText);
			if(Style::is_dark())
			{
				col_text.setAlpha(196);
			}

			QPen pen = painter->pen();
			pen.setColor(col_text);
			painter->setPen(pen);
		}
	}

	painter->translate(-4, 0);

	QFont font = option.font;
	{ // set the font
		if(GetSetting(Set::PL_FontSize) > 0)
		{
			font.setPointSize(GetSetting(Set::PL_FontSize));
		}

		else if(GetSetting(Set::Player_FontSize) > 0)
		{
			font.setPointSize(GetSetting(Set::Player_FontSize));
		}

		font.setWeight(QFont::Normal);
		painter->setFont(font);
		if(font.bold()){
			font.setWeight(PLAYLIST_BOLD);
		}

		painter->setFont(font);
	}

	painter->translate(4, 0);

	QString str;
	int offset_x = 4;

	for(int i=0; i<m->entry_look.size(); i++)
	{
		bool print = (i == m->entry_look.size() - 1);

		QChar c = m->entry_look.at(i);

		if( (c == '*') || (c == '\'') )
		{
			print = true;
		}

		else {
			str += c;
		}

		if(print)
		{
			QFontMetrics fm(font);
			painter->translate(offset_x, 0);

			str.replace("%title%", md.title());
			str.replace("%nr%", QString::number(md.track_number()));
			str.replace("%artist%", md.artist());
			str.replace("%album%", md.album());

			int flags = (Qt::AlignLeft);
			if(m->show_rating)
			{
				flags |= Qt::AlignTop;
				rect.setY(option.rect.y() + 2);
			}

			else {
				flags |= Qt::AlignVCenter;
			}

			if(md.radio_mode() != RadioMode::Station) {
				painter->drawText(rect, flags, fm.elidedText(str, Qt::ElideRight, rect.width()));
			}

			else
			{
				painter->drawText(rect, (Qt::AlignVCenter | Qt::AlignLeft), fm.elidedText(str, Qt::ElideRight, rect.width()));
			}

			offset_x = Gui::Util::text_width(fm, str);
			rect.setWidth(rect.width() - offset_x);
			str = "";
		}

		if(c == '*')
		{
			if(font.weight() == PLAYLIST_BOLD){
				font.setWeight(QFont::Normal);
			}

			else {
				font.setWeight(PLAYLIST_BOLD);
			}
			painter->setFont(font);
		}

		else if(c == '\''){
			font.setItalic(!font.italic());
			painter->setFont(font);
		}
	}


	if(m->show_rating)
	{
		painter->restore();
		painter->save();

		if(md.radio_mode() != RadioMode::Station)
		{
			painter->translate(0, 2);

			RatingLabel rating_label(nullptr, true);
			rating_label.set_rating(md.rating());
			rating_label.set_vertical_offset(option.rect.height() - m->rating_height);
			rating_label.paint(painter, option.rect);
		}
	}

	painter->restore();
}


void Delegate::sl_look_changed()
{
	m->entry_look = GetSetting(Set::PL_EntryLook);
}

void Delegate::sl_show_rating_changed()
{
	m->show_rating = GetSetting(Set::PL_ShowRating);
}

QWidget* Delegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option)

	Rating rating = index.data(Qt::EditRole).value<Rating>();
	if(rating == Rating::Last) {
		return nullptr;
	}

	auto* editor = new RatingEditor(rating, parent);
	editor->set_vertical_offset(option.rect.height() - m->rating_height);

	connect(editor, &RatingEditor::sig_finished, this, &Delegate::destroy_editor);

	return editor;
}


void Delegate::destroy_editor(bool save)
{
	Q_UNUSED(save)

	auto* editor = qobject_cast<RatingEditor*>(sender());
	if(!editor) {
		return;
	}

	disconnect(editor, &RatingEditor::sig_finished, this, &Delegate::destroy_editor);

	emit commitData(editor);
	emit closeEditor(editor);
}


void Delegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto* rating_editor = qobject_cast<RatingEditor*>(editor);
	if(!rating_editor) {
		return;
	}

	Rating rating = index.data(Qt::EditRole).value<Rating>();
	rating_editor->set_rating(rating);
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
