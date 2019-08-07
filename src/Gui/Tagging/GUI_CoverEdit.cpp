/* GUI_CoverEdit.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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



#include "GUI_CoverEdit.h"
#include "GUI_TagEdit.h"
#include "Gui/TagEdit/ui_GUI_CoverEdit.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Tagging/Editor.h"

#include "Utils/globals.h"
#include "Utils/Language/Language.h"
#include "Utils/Tagging/TaggingCover.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QMap>
#include <QPixmap>

using namespace Tagging;
using CoverPathMap=QMap<int, QPixmap>;

struct GUI_CoverEdit::Private
{
	Editor*				tag_edit=nullptr;
	CoverPathMap		index_cover_map;
	int					cur_idx;

	Private(Editor* editor) :
		tag_edit(editor),
		cur_idx(0)
	{}
};

GUI_CoverEdit::GUI_CoverEdit(GUI_TagEdit* parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>(parent->editor());
	ui = new Ui::GUI_CoverEdit();
	ui->setupUi(this);
	ui->btn_cover_replacement->set_silent(true);

	QString style = QString("min-width: %1ex; min-height: %1ex; max-width: %1ex; max-height: %1ex;").arg(20);
	ui->lab_cover_original->setStyleSheet(style);
	ui->btn_cover_replacement->setStyleSheet(style);

	connect(m->tag_edit, &Editor::sig_metadata_received, this, &GUI_CoverEdit::set_metadata);
	connect(ui->cb_cover_all, &QCheckBox::toggled, this, &GUI_CoverEdit::cover_all_toggled);
	connect(ui->btn_search, &QPushButton::clicked, ui->btn_cover_replacement, &Gui::CoverButton::trigger);
	connect(ui->btn_replace, &QPushButton::toggled, this, &GUI_CoverEdit::replace_toggled);
	connect(ui->btn_cover_replacement, &Gui::CoverButton::sig_cover_changed, this, &GUI_CoverEdit::cover_changed);

	language_changed();
}

GUI_CoverEdit::~GUI_CoverEdit() = default;

void GUI_CoverEdit::reset()
{
	ui->btn_replace->setChecked(false);
	ui->cb_cover_all->setChecked(false);
	ui->btn_cover_replacement->setEnabled(true);
	show_replacement_field(false);

	ui->btn_cover_replacement->set_cover_location(Cover::Location());

	m->index_cover_map.clear();
}

static void refresh_all_checkbox_text(QCheckBox* cb, int count)
{
	QString text = QString("%1 (%2 %3)")
		.arg(Lang::get(Lang::All))
		.arg(count)
		.arg(Lang::get(Lang::Tracks));

	cb->setText(text);
	cb->setEnabled(count > 0);
}

void GUI_CoverEdit::set_metadata(const MetaDataList& v_md)
{
	Q_UNUSED(v_md)

	refresh_current_track();
	refresh_all_checkbox_text(ui->cb_cover_all, v_md.count());
}

void GUI_CoverEdit::set_current_index(int index)
{
	m->cur_idx = index;
}

QPixmap GUI_CoverEdit::selected_cover(int index) const
{
	if(!is_cover_replacement_active()) {
		return QPixmap();
	}

	QPixmap pm;
	if(ui->cb_cover_all->isChecked()) {
		pm = m->index_cover_map[m->cur_idx];
	}

	else {
		pm = m->index_cover_map[index];
	}

	return pm;
}

void GUI_CoverEdit::refresh_current_track()
{
	if(m->cur_idx < 0 || m->cur_idx >= m->tag_edit->count()) {
		return;
	}

	MetaData md = m->tag_edit->metadata(m->cur_idx);
	set_cover(md);

	if(!ui->cb_cover_all->isChecked())
	{
		bool has_replacement = m->tag_edit->has_cover_replacement(m->cur_idx);
		ui->btn_cover_replacement->setChecked(has_replacement);
	}
}

void GUI_CoverEdit::show_replacement_field(bool b)
{
	ui->widget_replace->setVisible(b);
	ui->cb_cover_all->setChecked(false);
}


void GUI_CoverEdit::set_cover(const MetaData& md)
{
	QSize sz(this->height() / 2, this->height() / 2);
	ui->lab_cover_original->setFixedSize(sz);
	ui->btn_cover_replacement->setFixedSize(sz);

	bool has_cover = Tagging::Covers::has_cover(md.filepath());

	if(!has_cover)
	{
		ui->lab_cover_original->clear();
		ui->lab_cover_original->setText(tr("File has no cover"));
	}

	else
	{
		QPixmap pm = Tagging::Covers::extract_cover(md.filepath());
		if(pm.isNull())
		{
			ui->lab_cover_original->clear();
			ui->lab_cover_original->setText(tr("File has no cover"));
		}

		else
		{
			QPixmap pm_scaled = pm.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);

			ui->lab_cover_original->setPixmap(pm_scaled);
			ui->lab_cover_original->setText(QString());
		}
	}

	Cover::Location cl = Cover::Location::cover_location(md);

	ui->btn_cover_replacement->set_cover_location(cl);
	ui->btn_cover_replacement->setEnabled(cl.is_valid() && !ui->cb_cover_all->isChecked());

	ui->cb_cover_all->setEnabled(cl.is_valid());
}

void GUI_CoverEdit::replace_toggled(bool b)
{
	show_replacement_field(b);
}

void GUI_CoverEdit::cover_all_toggled(bool b)
{
	if(!b)
	{
		if(Util::between(m->cur_idx, m->tag_edit->count()) ) {
			set_cover(m->tag_edit->metadata(m->cur_idx));
		}
	}

	ui->btn_cover_replacement->setEnabled(!b);
	ui->btn_search->setEnabled(!b);
}

bool GUI_CoverEdit::is_cover_replacement_active() const
{
	return (ui->btn_replace->isChecked() &&
			ui->btn_cover_replacement->isVisible());
}

void GUI_CoverEdit::language_changed()
{
	refresh_all_checkbox_text(ui->cb_cover_all, m->tag_edit->count());
	ui->lab_original->setText(tr("Original"));
	ui->btn_search->setText(Lang::get(Lang::SearchVerb));
	ui->lab_replacement->setText(Lang::get(Lang::Replace));
	ui->btn_replace->setText(Lang::get(Lang::Replace));
}

void GUI_CoverEdit::cover_changed()
{
	QPixmap pm = ui->btn_cover_replacement->pixmap();
	m->index_cover_map[m->cur_idx] = pm;
}
