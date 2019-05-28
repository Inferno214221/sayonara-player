/* GUI_AudioConverter.cpp */

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

#include "GUI_AudioConverter.h"
#include "Gui/Plugins/ui_GUI_AudioConverter.h"

#include "Utils/Utils.h"
#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include "Gui/Utils/Style.h"

#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/AbstractPlaylist.h"
#include "Components/Converter/OggConverter.h"
#include "Components/Converter/LameConverter.h"

#include <QFileDialog>
#include <QStringList>

enum StackedWidgetPage
{
	Ogg=0,
	LameCBR,
	LameVBR
};

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

	QString preferred_converter = GetSetting(Set::AudioConvert_PreferredConverter);
	int idx = std::max(ui->combo_codecs->findText(preferred_converter), 0);

	ui->combo_codecs->setCurrentIndex(idx);
	ui->sw_preferences->setCurrentIndex(idx);

	ui->sw_progress->setCurrentIndex(0);
	ui->sb_threads->setValue(GetSetting(Set::AudioConvert_NumberThreads));

	int lame_cbr = GetSetting(Set::AudioConvert_QualityLameCBR);
	ui->combo_cbr->setCurrentIndex(ui->combo_cbr->findText(QString::number(lame_cbr)));

	ui->sb_lame_vbr->setValue(GetSetting(Set::AudioConvert_QualityLameVBR));
	ui->sb_ogg_quality->setValue(GetSetting(Set::AudioConvert_QualityOgg));

	connect(ui->btn_start, &QPushButton::clicked, this, &GUI_AudioConverter::btn_start_clicked);
	connect(ui->combo_codecs, combo_activated_int, this, &GUI_AudioConverter::combo_codecs_changed);
	connect(ui->combo_cbr, combo_activated_int, this, &GUI_AudioConverter::combo_cbr_lame_changed);

	connect(ui->sb_ogg_quality, spinbox_value_changed_int, this, &GUI_AudioConverter::ogg_quality_changed);
	connect(ui->sb_lame_vbr, spinbox_value_changed_int, this, &GUI_AudioConverter::lame_vbr_changed);
	connect(ui->sb_threads, spinbox_value_changed_int, this, &GUI_AudioConverter::num_threads_changed);

	check_start_button();
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
	int idx_codecs = ui->combo_codecs->currentIndex();
	int idx_cbr = ui->combo_cbr->currentIndex();

	ui->retranslateUi(this);
	ui->lab_threads->setText("#" + tr("Threads"));

	ui->combo_codecs->setCurrentIndex(idx_codecs);
	ui->combo_cbr->setCurrentIndex(idx_cbr);

	check_start_button();
}

void GUI_AudioConverter::btn_start_clicked()
{
	if(!is_ui_initialized()){
		return;
	}

	int n_threads = GetSetting(Set::AudioConvert_NumberThreads);

	PlaylistConstPtr pl = Playlist::Handler::instance()->playlist(Playlist::Handler::instance()->current_index());
	MetaDataList v_md = pl->metadata();

	Converter* converter = create_converter();
	{ // create and check converter
		if(!converter) {
			return;
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
			QString error_text = tr("Playlist does not contain tracks which are supported by the converter");
			Message::error
			(
				error_text +
				QString(" (%1). ").arg(converter->supported_input_formats().join(", ")) +
				tr("No track will be converted.")
			);

			converter->deleteLater();
			return;
		}

		if(converter->num_files() < v_md.count())
		{
			QString error_text = tr("Playlist does not contain tracks which are supported by the converter");
			Message::warning
			(
				error_text +
				QString(" (%1). ").arg(converter->supported_input_formats().join(", ")) +
				tr("These tracks will be ignored")
			);
		}
	}

	QString dir;
	{ // set target dir
		QString cvt_target_path = GetSetting(Set::Engine_CovertTargetPath);
		dir = QFileDialog::getExistingDirectory(this, "Choose target directory", cvt_target_path);
		if(dir.isEmpty()){
			converter->deleteLater();
			return;
		}
		SetSetting(Set::Engine_CovertTargetPath, dir);
	}

	connect(converter, &OggConverter::sig_finished, this, &GUI_AudioConverter::convert_finished);
	connect(converter, &OggConverter::sig_progress, ui->pb_progress, &QProgressBar::setValue);
	connect(ui->btn_stop_encoding, &QPushButton::clicked, converter, &OggConverter::stop);
	connect(ui->btn_stop_encoding, &QPushButton::clicked, this, &GUI_AudioConverter::reset_buttons);

	ui->pb_progress->setValue(0);
	ui->sw_progress->setCurrentIndex(1);

	converter->start(n_threads, dir);
}

void GUI_AudioConverter::convert_finished()
{
	OggConverter* converter = static_cast<OggConverter*>(sender());

	if(converter)
	{
		Message::error( QStringList
		{
			tr("Failed to convert %1 tracks").arg(converter->num_errors()),
			tr("Please check the log files") + ". " + Util::create_link(converter->log_directory(), Style::is_dark(), "file://" + converter->log_directory())
		}.join("<br>"));

		converter->deleteLater();
	}

	else {
		Message::info(tr("Successfully finished"));
	}

	reset_buttons();
}

void GUI_AudioConverter::combo_codecs_changed(int idx)
{
	if(idx < 0){
		return;
	}

	ui->sw_preferences->setCurrentIndex(idx);

	QString text = ui->combo_codecs->currentText();
	SetSetting(Set::AudioConvert_PreferredConverter, text);

	check_start_button();
}

void GUI_AudioConverter::reset_buttons()
{
	ui->sw_progress->setCurrentIndex(0);
}

void GUI_AudioConverter::num_threads_changed(int value)
{
	SetSetting(Set::AudioConvert_NumberThreads, value);
}

void GUI_AudioConverter::check_start_button()
{
	Converter* converter = create_converter();

	if(!converter->is_available()){
		ui->btn_start->setEnabled(false);
		ui->btn_start->setText(tr("Cannot find encoder") + (" '" + converter->binary() + "'"));
	}

	else {
		ui->btn_start->setEnabled(true);
		ui->btn_start->setText(tr("Start"));
	}

	converter->deleteLater();
}

Converter* GUI_AudioConverter::create_converter()
{
	Converter* converter=nullptr;
	switch(ui->sw_preferences->currentIndex())
	{
		case StackedWidgetPage::Ogg:
			converter = new OggConverter(ui->sb_ogg_quality->value(), this);
			break;
		case StackedWidgetPage::LameCBR:
			converter = new LameConverter(true, ui->combo_cbr->currentText().toInt(), this );
			break;
		case StackedWidgetPage::LameVBR:
			converter = new LameConverter(false, ui->sb_lame_vbr->value(), this );
			break;
	}

	return converter;
}

void GUI_AudioConverter::ogg_quality_changed(int value)
{
	SetSetting(Set::AudioConvert_QualityOgg, value);
}

void GUI_AudioConverter::lame_vbr_changed(int value)
{
	SetSetting(Set::AudioConvert_QualityLameVBR, value);
}

void GUI_AudioConverter::combo_cbr_lame_changed(int idx)
{
	Q_UNUSED(idx)
	SetSetting(Set::AudioConvert_QualityLameCBR, ui->combo_cbr->currentText().toInt());
}

