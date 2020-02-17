/* TagFromPath.cpp */

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

#include "GUI_TagFromPath.h"

#include "Gui/TagEdit/ui_GUI_TagFromPath.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Message/Message.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

#include <QDesktopServices>
#include <QMap>

using namespace Tagging;

struct GUI_TagFromPath::Private
{
	QString							currentFilepath;
	QMap<TagName, ReplacedString>	tagReplaceStringMap;
};

GUI_TagFromPath::GUI_TagFromPath(QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>();
	ui = new Ui::GUI_TagFromPath();
	ui->setupUi(this);

	showErrorFrame(false);

	connect(ui->btn_apply_tag, &QPushButton::clicked, this, &GUI_TagFromPath::sigApply);
	connect(ui->btn_apply_tag_all, &QPushButton::clicked, this, &GUI_TagFromPath::sigApplyAll);

	connect(ui->le_tag, &QLineEdit::textChanged, this, &GUI_TagFromPath::tagTextChanged);

	connect(ui->btn_title, &QPushButton::toggled, this, &GUI_TagFromPath::btnTitleChecked);
	connect(ui->btn_artist, &QPushButton::toggled, this, &GUI_TagFromPath::btnArtistChecked);
	connect(ui->btn_album, &QPushButton::toggled, this, &GUI_TagFromPath::btnAlbumChecked);
	connect(ui->btn_track_nr, &QPushButton::toggled, this, &GUI_TagFromPath::btnTrackNrChecked);
	connect(ui->btn_year, &QPushButton::toggled, this, &GUI_TagFromPath::btnYearChecked);
	connect(ui->btn_disc_nr, &QPushButton::toggled, this, &GUI_TagFromPath::btnDiscnumberChecked);
	connect(ui->btn_tag_help, &QPushButton::clicked, this, &GUI_TagFromPath::btnTagHelpClicked);

	languageChanged();
}

GUI_TagFromPath::~GUI_TagFromPath() = default;

void GUI_TagFromPath::setFilepath(const QString& filepath)
{
	m->currentFilepath = filepath;

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
	setTagColors(valid);

	Tagging::TagType tag_type = Tagging::Utils::getTagType(filepath);
	QString tag_type_string = Tagging::Utils::tagTypeToString(tag_type);

	ui->lab_tag_type->setText(tr("Tag") + ": " + tag_type_string);
}


void GUI_TagFromPath::reset()
{
	ui->le_tag->clear();
	ui->le_tag->setEnabled(true);
	ui->lv_invalidFilepaths->clear();

	ui->btn_album->setChecked(false);
	ui->btn_artist->setChecked(false);
	ui->btn_title->setChecked(false);
	ui->btn_year->setChecked(false);
	ui->btn_disc_nr->setChecked(false);
	ui->btn_track_nr->setChecked(false);
}

void GUI_TagFromPath::setTagColors(bool valid)
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


void GUI_TagFromPath::tagTextChanged(const QString& tag_string)
{
	Expression e(tag_string, m->currentFilepath);
	setTagColors(e.is_valid());
}


void GUI_TagFromPath::clearInvalidFilepaths()
{
	showErrorFrame(false);
	ui->lv_invalidFilepaths->clear();
}

void GUI_TagFromPath::addInvalidFilepath(const QString& filepath)
{
	showErrorFrame(true);
	ui->lv_invalidFilepaths->addItem(filepath);
}

bool GUI_TagFromPath::replaceSelectedTagText(TagName tag_name, bool b)
{
	TagLineEdit::TextSelection ts = ui->le_tag->textSelection();

	if(ts.selectionStart < 0 && b)
	{
		spLog(Log::Debug, this) << "Nothing selected...";

		Message::info(tr("Please select text first"));
		return false;
	}

	QString text = ui->le_tag->text();
	QString tag_string = Tagging::tag_name_to_string(tag_name);

	// replace the string by a tag
	if(b)
	{
		ReplacedString selected_text = text.mid( ts.selectionStart, ts.selectionSize );

		text.replace(ts.selectionStart, ts.selectionSize, tag_string);
		ui->le_tag->setText(text);

		m->tagReplaceStringMap[tag_name] = selected_text;
	}

	// replace tag by the original string
	else
	{
		text.replace(tag_string, m->tagReplaceStringMap[tag_name]);
		ui->le_tag->setText(text);

		m->tagReplaceStringMap.remove(tag_name);
	}

	Expression e(text, m->currentFilepath);
	setTagColors(e.is_valid());

	return true;
}

void GUI_TagFromPath::btnChecked(QPushButton* btn, bool b, TagName tag_name)
{
	ui->lab_tag_from_path_warning->setVisible(false);
	ui->lv_invalidFilepaths->setVisible(false);

	if(!replaceSelectedTagText(tag_name, b)){
		btn->setChecked(false);
	}
}

void GUI_TagFromPath::showErrorFrame(bool b)
{
	ui->sw_tag_from_path->setCurrentIndex((b == true) ? 0 : 1);
}

void GUI_TagFromPath::btnTitleChecked(bool b)
{
	btnChecked(ui->btn_title, b, TagTitle);
}

void GUI_TagFromPath::btnArtistChecked(bool b)
{
	btnChecked(ui->btn_artist, b, TagArtist);
}

void GUI_TagFromPath::btnAlbumChecked(bool b)
{
	btnChecked(ui->btn_album, b, TagAlbum);
}

void GUI_TagFromPath::btnTrackNrChecked(bool b)
{
	btnChecked(ui->btn_track_nr, b, TagTrackNum);
}

void GUI_TagFromPath::btnDiscnumberChecked(bool b)
{
	btnChecked(ui->btn_disc_nr, b, TagDisc);
}

void GUI_TagFromPath::btnYearChecked(bool b)
{
	btnChecked(ui->btn_year, b, TagYear);
}

void GUI_TagFromPath::btnTagHelpClicked()
{
	QUrl url(QString("http://sayonara-player.com/faq.php#tag-edit"));
	QDesktopServices::openUrl(url);
}

void GUI_TagFromPath::languageChanged()
{
	ui->retranslateUi(this);

	ui->btn_title->setText(Lang::get(Lang::Title));
	ui->btn_album->setText(Lang::get(Lang::Album));
	ui->btn_artist->setText(Lang::get(Lang::Artist));
	ui->btn_year->setText(Lang::get(Lang::Year));
	ui->btn_track_nr->setText(Lang::get(Lang::TrackNo).toFirstUpper());
	ui->lab_tag_from_path_warning->setText(Lang::get(Lang::Warning));

	ui->btn_apply_tag_all->setText(Lang::get(Lang::Apply) + ": " + Lang::get(Lang::All).toFirstUpper());
	ui->btn_apply_tag->setText(Lang::get(Lang::Apply) + ": " + Lang::get(Lang::Title).toFirstUpper());
}

QString GUI_TagFromPath::getRegexString() const
{
	return ui->le_tag->text();
}
