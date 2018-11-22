/* GUI_AudioConverter.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "GUI_AudioConverter.h"
#include "GUI/Plugins/ui_GUI_AudioConverter.h"

#include "Utils/Utils.h"
#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include "GUI/Utils/Style.h"

#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/AbstractPlaylist.h"
#include "Components/Converter/OggConverter.h"
#include "Components/Converter/LameConverter.h"

#include <QFileDialog>
#include <QStringList>

struct GUI_AudioConverter::Private
{
	bool				mp3_enc_available;

	Private() :
		mp3_enc_available(true)
	{}
};

GUI_AudioConverter::GUI_AudioConverter(QWidget *parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>();
}


GUI_AudioConverter::~GUI_AudioConverter()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}


void GUI_AudioConverter::init_ui()
{
	setup_parent(this, &ui);

	ui->pb_progress->setVisible(false);
	ui->stackedWidget->setCurrentIndex(0);
	ui->rb_cbr->setChecked(true);
	ui->btn_stop_encoding->setVisible(false);

	connect(ui->btn_start, &QPushButton::clicked, this, &GUI_AudioConverter::btn_start_clicked);
	connect(ui->combo_codecs, &QComboBox::currentTextChanged, this, &GUI_AudioConverter::combo_codecs_changed);
}


QString GUI_AudioConverter::get_name() const
{
	return "Audio Converter";
}


QString GUI_AudioConverter::get_display_name() const
{
	return tr("Audio Converter");
}


void GUI_AudioConverter::retranslate_ui()
{
	ui->retranslateUi(this);
}

void GUI_AudioConverter::btn_start_clicked()
{
	if(!is_ui_initialized()){
		return;
	}

	int n_threads = 8;

	PlaylistConstPtr pl = Playlist::Handler::instance()->playlist(Playlist::Handler::instance()->current_index());
	MetaDataList v_md = pl->metadata();

	Converter* converter;
	{ // create and check converter
		if(ui->stackedWidget->currentIndex() == 0)
		{
			converter = new OggConverter(n_threads, ui->sb_ogg_quality->value(), this);
		}

		else
		{
			if(ui->rb_cbr->isChecked()){
				converter = new LameConverter(n_threads, ui->rb_cbr->isChecked(), ui->combo_cbr->currentText().toInt(), this );
			}

			else{
				converter = new LameConverter(n_threads, ui->rb_cbr->isChecked(), ui->combo_vbr->currentText().toInt(), this );
			}
		}

		if(!converter->is_available())
		{
			Message::error(tr("Cannot find encoder") + (" '" + converter->binary() + "'") );
			converter->deleteLater();
			return;
		}
	}

	{ // check input files
		converter->add_metadata(v_md);
		if(converter->num_files() == 0)
		{
			QString error_text = tr("Playlist does not contain files supported by the converter");
			Message::error
			(
				error_text +
				QString(" (%1). ").arg(converter->supported_input_formats().join(", ")) +
				tr("No file will be converted.")
			);

			converter->deleteLater();
			return;
		}

		if(converter->num_files() < v_md.count())
		{
			QString error_text = tr("Playlist does not contain files supported by the converter");
			Message::warning
			(
				error_text +
				QString(" (%1). ").arg(converter->supported_input_formats().join(", ")) +
				tr("These files will be ignored")
			);
		}
	}

	QString dir;
	{ // set target dir
		QString cvt_target_path = _settings->get<Set::Engine_CovertTargetPath>();
		dir = QFileDialog::getExistingDirectory(this, "Choose target directory", cvt_target_path);
		if(dir.isEmpty()){
			converter->deleteLater();
			return;
		}
		_settings->set<Set::Engine_CovertTargetPath>(dir);
	}

	connect(converter, &OggConverter::sig_finished, this, &GUI_AudioConverter::convert_finished);
	connect(converter, &OggConverter::sig_progress, ui->pb_progress, &QProgressBar::setValue);
	connect(ui->btn_stop_encoding, &QPushButton::clicked, converter, &OggConverter::stop);

	ui->pb_progress->setVisible(true);
	ui->pb_progress->setValue(0);
	ui->btn_start->setVisible(false);
	ui->btn_stop_encoding->setVisible(true);

	converter->start(dir);
}

void GUI_AudioConverter::convert_finished()
{
	OggConverter* converter = static_cast<OggConverter*>(sender());

	if(converter && converter->num_errors() != 100){
		Message::error( QStringList
		{
			tr("Failed to convert %1 tracks").arg(converter->num_errors()),
			tr("Please check the log files") + ". " + Util::create_link(converter->log_directory(), Style::is_dark(), "file://" + converter->log_directory())
		}.join("<br>"));
	}

	else {
		Message::info(tr("Successfully finished"));
	}

	ui->pb_progress->setVisible(false);
	ui->btn_start->setVisible(true);
	ui->btn_stop_encoding->setVisible(false);

	converter->deleteLater();
}

void GUI_AudioConverter::combo_codecs_changed(const QString& text)
{
	if(text.toLower().contains("mp3") || text.toLower().contains("lame")){
		ui->stackedWidget->setCurrentIndex(1);
	}

	else {
		ui->stackedWidget->setCurrentIndex(0);
	}
}

void GUI_AudioConverter::mp3_enc_found()
{
	m->mp3_enc_available = _settings->get<SetNoDB::MP3enc_found>();
}
