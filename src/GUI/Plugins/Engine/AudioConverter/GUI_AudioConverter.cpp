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

#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Playlist/PlaylistMode.h"

#include "Components/Engine/EngineHandler.h"
#include "Components/Engine/Convert/LameBitrate.h"
#include "Components/PlayManager/PlayManager.h"

#include <QFileDialog>

struct GUI_AudioConverter::Private
{
	Playlist::Mode		pl_mode;
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

	LameBitrate br = (LameBitrate) _settings->get<Set::Engine_ConvertQuality>();

	ui->rb_cbr->setChecked(false);
	ui->rb_vbr->setChecked(false);

	connect(ui->rb_cbr, &QRadioButton::toggled, this, &GUI_AudioConverter::rb_cbr_toggled);
	connect(ui->rb_vbr, &QRadioButton::toggled, this, &GUI_AudioConverter::rb_vbr_toggled);
	connect(ui->cb_quality,	combo_current_index_changed_int, this, &GUI_AudioConverter::quality_changed);
	connect(ui->cb_active, &QCheckBox::toggled, this, &GUI_AudioConverter::cb_active_toggled);

	int idx = -1;

	switch(br) {
		case LameBitrate_64:
		case LameBitrate_128:
		case LameBitrate_192:
		case LameBitrate_256:
		case LameBitrate_320:
			ui->rb_cbr->setChecked(true);
			idx = ui->cb_quality->findData((int) br);
			break;

		default:
			ui->rb_vbr->setChecked(true);
			idx = ui->cb_quality->findData((int) br);
			break;
	}

	if(idx >= 0 && idx < ui->cb_quality->count()) {
		ui->cb_quality->setCurrentIndex(idx);
	}

	PlayManagerPtr play_manager = PlayManager::instance();
	connect(play_manager, &PlayManager::sig_playstate_changed, this, &GUI_AudioConverter::playstate_changed);

	Set::listen<SetNoDB::MP3enc_found>(this, &GUI_AudioConverter::mp3_enc_found);
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
	ui->cb_active->setText(Lang::get(Lang::Active));
}


void GUI_AudioConverter::fill_cbr()
{
	if(!is_ui_initialized()){
		return;
	}

	disconnect(ui->cb_quality,	combo_current_index_changed_int, this, &GUI_AudioConverter::quality_changed);

	ui->cb_quality->clear();
	ui->cb_quality->addItem("64", LameBitrate_64);
	ui->cb_quality->addItem("128", LameBitrate_128);
	ui->cb_quality->addItem("192", LameBitrate_192);
	ui->cb_quality->addItem("256", LameBitrate_256);
	ui->cb_quality->addItem("320", LameBitrate_320);

	connect(ui->cb_quality,	combo_current_index_changed_int, this, &GUI_AudioConverter::quality_changed);

	ui->cb_quality->setCurrentIndex(2);
}


void GUI_AudioConverter::fill_vbr()
{
	if(!is_ui_initialized()){
		return;
	}

	disconnect(ui->cb_quality,	combo_current_index_changed_int, this, &GUI_AudioConverter::quality_changed);

	ui->cb_quality->clear();
	ui->cb_quality->addItem("0 (" + tr("Best") + ")", LameBitrate_var_0);
	ui->cb_quality->addItem("1", LameBitrate_var_1);
	ui->cb_quality->addItem("2", LameBitrate_var_2);
	ui->cb_quality->addItem("3", LameBitrate_var_3);
	ui->cb_quality->addItem("4", LameBitrate_var_4);
	ui->cb_quality->addItem("5", LameBitrate_var_5);
	ui->cb_quality->addItem("6", LameBitrate_var_6);
	ui->cb_quality->addItem("7", LameBitrate_var_7);
	ui->cb_quality->addItem("8", LameBitrate_var_8);
	ui->cb_quality->addItem("9 (" + tr("Worst") + ")", LameBitrate_var_9);

	connect(ui->cb_quality,	combo_current_index_changed_int, this, &GUI_AudioConverter::quality_changed);

	ui->cb_quality->setCurrentIndex(2);
}


void GUI_AudioConverter::playstate_changed(PlayState state)
{
	if(state == PlayState::Stopped){
		stopped();
	}
}


void GUI_AudioConverter::stopped()
{
	if(!isVisible()) {
		return;
	}

	if(!ui->cb_active->isChecked()) {
		return;
	}

	ui->cb_active->setChecked( false );
}


void GUI_AudioConverter::rb_cbr_toggled(bool b)
{
	if(!is_ui_initialized()){
		return;
	}

	if(b) {
		fill_cbr();
	}
}

void GUI_AudioConverter::rb_vbr_toggled(bool b)
{
	if(!is_ui_initialized()){
		return;
	}

	if(b) {
		fill_vbr();
	}
}

void GUI_AudioConverter::pl_mode_backup()
{
	m->pl_mode = _settings->get<Set::PL_Mode>();

	Playlist::Mode new_mode;
		new_mode.setAppend(false, false);
		new_mode.setRep1(false, false);
		new_mode.setRepAll(false, false);
		new_mode.setShuffle(false, false);
		new_mode.setGapless(false, false);
		new_mode.setDynamic(false, false);

	_settings->set<Set::PL_Mode>(new_mode);
}

void GUI_AudioConverter::pl_mode_restore()
{
	_settings->set<Set::PL_Mode>(m->pl_mode);
}

void GUI_AudioConverter::cb_active_toggled(bool b)
{
	if(!is_ui_initialized()){
		return;
	}

	if(!m->mp3_enc_available)
	{
		Message::warning(Lang::get(Lang::CannotFindLame));

		disconnect(ui->cb_active, &QCheckBox::toggled, this, &GUI_AudioConverter::cb_active_toggled);
		ui->cb_active->setChecked(false);
		connect(ui->cb_active, &QCheckBox::toggled, this, &GUI_AudioConverter::cb_active_toggled);

		return;
	}

	Engine::Handler* engine = Engine::Handler::instance();
	if(b) {
		QString cvt_target_path = _settings->get<Set::Engine_CovertTargetPath>();
		QString dir = QFileDialog::getExistingDirectory(this, "Choose target directory", cvt_target_path);

		if(dir.size() > 0) {
			_settings->set<Set::Engine_CovertTargetPath>(dir);
			pl_mode_backup();

			engine->start_convert();
		}

		else {
			disconnect(ui->cb_active, &QCheckBox::toggled, this, &GUI_AudioConverter::cb_active_toggled);
			ui->cb_active->setChecked(false);
			connect(ui->cb_active, &QCheckBox::toggled, this, &GUI_AudioConverter::cb_active_toggled);
		}
	}

	else {
		pl_mode_restore();
		engine->end_convert();
	}
}

void GUI_AudioConverter::quality_changed(int index)
{
	if(!is_ui_initialized()){
		return;
	}

	LameBitrate q = (LameBitrate) ui->cb_quality->itemData(index).toInt();
	sp_log(Log::Info, this) << "Quality: " << q;
	_settings->set<Set::Engine_ConvertQuality>((int) q);
}

void GUI_AudioConverter::mp3_enc_found()
{
	m->mp3_enc_available = _settings->get<SetNoDB::MP3enc_found>();
}
