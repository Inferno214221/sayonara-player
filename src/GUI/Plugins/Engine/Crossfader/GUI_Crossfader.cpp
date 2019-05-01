/* GUI_Crossfader.cpp */

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

#include "GUI_Crossfader.h"
#include "GUI/Plugins/ui_GUI_Crossfader.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"


GUI_Crossfader::GUI_Crossfader(QWidget *parent) :
	PlayerPlugin::Base(parent) {}

GUI_Crossfader::~GUI_Crossfader()
{
	if(ui)
	{
		delete ui; ui = nullptr;
	}
}


void GUI_Crossfader::init_ui()
{
	setup_parent(this, &ui);

	Playlist::Mode mode = GetSetting(Set::PL_Mode);

	bool gapless_active = Playlist::Mode::isActive(mode.gapless());
	bool crossfader_active = GetSetting(Set::Engine_CrossFaderActive);

	int val = GetSetting(Set::Engine_CrossFaderTime);

	if(gapless_active && crossfader_active){
		gapless_active = false;
	}

	ui->cb_gapless->setChecked(gapless_active);
	ui->cb_crossfader->setChecked(crossfader_active);
	ui->sli_crossfader->setValue(val);
	ui->lab_crossfader->setText(QString::number(val) + " ms");

	crossfader_active_changed(crossfader_active);
	gapless_active_changed(gapless_active);

	connect(ui->cb_crossfader, &QCheckBox::clicked, this, &GUI_Crossfader::crossfader_active_changed);
	connect(ui->cb_gapless, &QCheckBox::clicked, this, &GUI_Crossfader::gapless_active_changed);

	connect(ui->sli_crossfader, &QSlider::valueChanged, this, &GUI_Crossfader::slider_changed);
}


void GUI_Crossfader::retranslate_ui()
{
	ui->retranslateUi(this);

	ui->cb_crossfader->setText(tr("Crossfader"));
	ui->cb_gapless->setText(Lang::get(Lang::GaplessPlayback));
}

QString GUI_Crossfader::get_name() const
{
	return "Crossfader";
}

QString GUI_Crossfader::get_display_name() const
{
	return tr("Crossfader") + " / " + Lang::get(Lang::GaplessPlayback);
}


void GUI_Crossfader::slider_changed(int val)
{
	SetSetting(Set::Engine_CrossFaderTime, val);

	if(val == 0){
		ui->lab_crossfader->setText(Lang::get(Lang::GaplessPlayback));
	}

	else {
		ui->lab_crossfader->setText(QString::number(val) + "ms");
	}
}



void GUI_Crossfader::crossfader_active_changed(bool b)
{
	if(b)
	{
		ui->cb_gapless->setChecked(false);
		gapless_active_changed(!b);
	}

	ui->cb_crossfader->setChecked(b);
	ui->lab_crossfader->setEnabled(b);
	ui->sli_crossfader->setEnabled(b);

	SetSetting(Set::Engine_CrossFaderActive, b);
}


void GUI_Crossfader::gapless_active_changed(bool b)
{
	if(b)
	{
		ui->cb_crossfader->setChecked(false);
		ui->lab_crossfader->setEnabled(false);
		ui->sli_crossfader->setEnabled(false);

		crossfader_active_changed(!b);
	}

	ui->cb_gapless->setChecked(b);

	Playlist::Mode plm = GetSetting(Set::PL_Mode);
	plm.setGapless(b);

	SetSetting(Set::PL_Mode, plm);
}
