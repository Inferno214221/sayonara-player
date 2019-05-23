/* TagFromPath.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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



#include "TagFromPath.h"
#include "TextSelection.h"

#include "Gui/TagEdit/ui_GUI_TagFromPath.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Message/Message.h"
#include "Utils/Language.h"
#include "Utils/Logger/Logger.h"

#include <QDesktopServices>
#include <QMap>

using namespace Tagging;

struct GUI_TagFromPath::Private
{
	QString							current_filepath;
	QMap<TagName, ReplacedString>	tag_str_map;
};

GUI_TagFromPath::GUI_TagFromPath(QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>();
	ui = new Ui::GUI_TagFromPath();
	ui->setupUi(this);

	show_error_frame(false);

	connect(ui->btn_apply_tag, &QPushButton::clicked, this, &GUI_TagFromPath::sig_apply);
	connect(ui->btn_apply_tag_all, &QPushButton::clicked, this, &GUI_TagFromPath::sig_apply_all);

	connect(ui->le_tag, &QLineEdit::textChanged, this, &GUI_TagFromPath::tag_text_changed);

	connect(ui->btn_title, &QPushButton::toggled, this, &GUI_TagFromPath::btn_title_checked);
	connect(ui->btn_artist, &QPushButton::toggled, this, &GUI_TagFromPath::btn_artist_checked);
	connect(ui->btn_album, &QPushButton::toggled, this, &GUI_TagFromPath::btn_album_checked);
	connect(ui->btn_track_nr, &QPushButton::toggled, this, &GUI_TagFromPath::btn_track_nr_checked);
	connect(ui->btn_year, &QPushButton::toggled, this, &GUI_TagFromPath::btn_year_checked);
	connect(ui->btn_disc_nr, &QPushButton::toggled, this, &GUI_TagFromPath::btn_disc_nr_checked);
	connect(ui->btn_tag_help, &QPushButton::clicked, this, &GUI_TagFromPath::btn_tag_help_clicked);

	language_changed();
}

GUI_TagFromPath::~GUI_TagFromPath() {}


void GUI_TagFromPath::set_filepath(const QString& filepath)
{
	m->current_filepath = filepath;

	if(ui->le_tag->text().isEmpty()){
		ui->le_tag->setText(filepath);
	}

	else if( !(ui->btn_album->isChecked() ||
			ui->btn_artist->isChecked() ||
			ui->btn_title->isChecked() ||
			ui->btn_year->isChecked() ||
			ui->btn_disc_nr->isChecked() ||
			ui->btn_track_nr->isChecked()))
	{
		ui->le_tag->setText(filepath);
	}

	Expression e(ui->le_tag->text(), filepath);
	bool valid = e.is_valid();
	set_tag_colors(valid);

	Tagging::TagType tag_type = Tagging::Utils::get_tag_type(filepath);
	QString tag_type_string = Tagging::Utils::tag_type_to_string(tag_type);

	ui->lab_tag_type->setText(tr("Tag") + ": " + tag_type_string);
}


void GUI_TagFromPath::reset()
{
	ui->le_tag->clear();
	ui->le_tag->setEnabled(true);
	ui->lv_tag_from_path_files->clear();

	ui->btn_album->setChecked(false);
	ui->btn_artist->setChecked(false);
	ui->btn_title->setChecked(false);
	ui->btn_year->setChecked(false);
	ui->btn_disc_nr->setChecked(false);
	ui->btn_track_nr->setChecked(false);
}

void GUI_TagFromPath::set_tag_colors(bool valid)
{
	if( !valid ){
		ui->le_tag->setStyleSheet("font-family: mono; font-size: 12pt; color: red;");
	}

	else{
		ui->le_tag->setStyleSheet("font-family: mono; font-size: 12pt;");
	}

	ui->btn_apply_tag->setEnabled(valid);
	ui->btn_apply_tag_all->setEnabled(valid);
}


void GUI_TagFromPath::tag_text_changed(const QString& tag_string)
{
	Expression e(tag_string, m->current_filepath);
	set_tag_colors(e.is_valid());
}


void GUI_TagFromPath::clear_invalid_filepaths()
{
	show_error_frame(false);
	ui->lv_tag_from_path_files->clear();
}

void GUI_TagFromPath::add_invalid_filepath(const QString& filepath)
{
	show_error_frame(true);
	ui->lv_tag_from_path_files->addItem(filepath);
}


bool GUI_TagFromPath::replace_selected_tag_text(TagName tag_name, bool b)
{
	TextSelection ts = ui->le_tag->text_selection();

	if(ts.selection_start < 0 && b)
	{
		Message::info(tr("Please select text first"));

		sp_log(Log::Debug, this) << "Nothing selected...";

		return false;
	}

	QString text = ui->le_tag->text();

	// replace the string by a tag
	if(b)
	{
		ReplacedString selected_text = text.mid( ts.selection_start, ts.selection_size );
		QString tag_string = Tagging::tag_name_to_string(tag_name);

		text.replace(ts.selection_start, ts.selection_size, tag_string);
		ui->le_tag->setText(text);

		m->tag_str_map[tag_name] = selected_text;
	}

	// replace tag by the original string
	else
	{
		QString tag_string = Tagging::tag_name_to_string(tag_name);
		text.replace(tag_string, m->tag_str_map[tag_name]);
		ui->le_tag->setText(text);

		m->tag_str_map.remove(tag_name);
	}

	Expression e(text, m->current_filepath);
	set_tag_colors(e.is_valid());

	return true;
}

void GUI_TagFromPath::btn_checked(QPushButton* btn, bool b, TagName tag_name)
{
	if(!replace_selected_tag_text(tag_name, b)){
		btn->setChecked(false);
	}
}

void GUI_TagFromPath::show_error_frame(bool b)
{
	ui->sw_tag_from_path->setCurrentIndex((b == true) ? 0 : 1);
}

void GUI_TagFromPath::btn_title_checked(bool b)
{
	btn_checked(ui->btn_title, b, TagTitle);
}

void GUI_TagFromPath::btn_artist_checked(bool b)
{
	btn_checked(ui->btn_artist, b, TagArtist);
}

void GUI_TagFromPath::btn_album_checked(bool b)
{
	btn_checked(ui->btn_album, b, TagAlbum);
}

void GUI_TagFromPath::btn_track_nr_checked(bool b)
{
	btn_checked(ui->btn_track_nr, b, TagTrackNum);
}

void GUI_TagFromPath::btn_disc_nr_checked(bool b)
{
	btn_checked(ui->btn_disc_nr, b, TagDisc);
}

void GUI_TagFromPath::btn_year_checked(bool b)
{
	btn_checked(ui->btn_year, b, TagYear);
}

void GUI_TagFromPath::btn_tag_help_clicked()
{
	QUrl url(QString("http://sayonara-player.com/faq.php#tag-edit"));
	QDesktopServices::openUrl(url);
}

void GUI_TagFromPath::language_changed()
{
	ui->btn_title->setText(Lang::get(Lang::Title));
	ui->btn_album->setText(Lang::get(Lang::Album));
	ui->btn_artist->setText(Lang::get(Lang::Artist));
	ui->btn_year->setText(Lang::get(Lang::Year));
	ui->btn_track_nr->setText(Lang::get(Lang::TrackNo).toFirstUpper());
	ui->lab_tag_from_path_warning->setText(Lang::get(Lang::Warning));

	ui->btn_apply_tag_all->setText(Lang::get(Lang::Apply) + ": " + Lang::get(Lang::All).toFirstUpper());
	ui->btn_apply_tag->setText(Lang::get(Lang::Apply) + ": " + Lang::get(Lang::Title).toFirstUpper());
}

QString GUI_TagFromPath::get_regex_string() const
{
	return ui->le_tag->text();
}
