/* GUI_Crossfader.cpp */

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

#include "GUI_Crossfader.h"
#include "Gui/Plugins/ui_GUI_Crossfader.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

GUI_Crossfader::GUI_Crossfader(QWidget* parent) :
	PlayerPlugin::Base(parent) {}

GUI_Crossfader::~GUI_Crossfader()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

void GUI_Crossfader::initUi()
{
	setupParent(this, &ui);

	const auto playlistMode = GetSetting(Set::PL_Mode);
	const auto crossfaderActive = GetSetting(Set::Engine_CrossFaderActive);
	const auto gaplessActive = (!crossfaderActive)
	                           ? Playlist::Mode::isActiveAndEnabled(playlistMode.gapless())
	                           : false;

	const auto crossfaderTime = GetSetting(Set::Engine_CrossFaderTime);

	ui->cb_gapless->setChecked(gaplessActive);
	ui->cb_crossfader->setChecked(crossfaderActive);
	ui->sli_crossfader->setValue(crossfaderTime);
	ui->lab_crossfader->setText(QString::number(crossfaderTime) + " ms");

	crossfaderActiveChanged(crossfaderActive);
	gaplessActiveChanged(gaplessActive);

	connect(ui->cb_crossfader, &QCheckBox::clicked, this, &GUI_Crossfader::crossfaderActiveChanged);
	connect(ui->cb_gapless, &QCheckBox::clicked, this, &GUI_Crossfader::gaplessActiveChanged);
	connect(ui->sli_crossfader, &QSlider::valueChanged, this, &GUI_Crossfader::sliderChanged);

	/** No crossfader with alsa **/
	ListenSetting(Set::Engine_Sink, GUI_Crossfader::engineChanged);
}

void GUI_Crossfader::retranslate()
{
	ui->retranslateUi(this);

	ui->cb_crossfader->setText(tr("Crossfader"));
	ui->cb_gapless->setText(Lang::get(Lang::GaplessPlayback));
}

QString GUI_Crossfader::name() const
{
	return "Crossfader";
}

QString GUI_Crossfader::displayName() const
{
	const auto gaplessPlaybackString = Lang::get(Lang::GaplessPlayback);
	const auto crossfaderString = tr("Crossfader");

	return tr("%1 and %2")
		.arg(gaplessPlaybackString)
		.arg(crossfaderString);
}

void GUI_Crossfader::sliderChanged(int val)
{
	SetSetting(Set::Engine_CrossFaderTime, val);

	const auto text = (val == 0)
	                  ? Lang::get(Lang::GaplessPlayback)
	                  : QString("%1 ms").arg(val);

	ui->lab_crossfader->setText(text);
}

void GUI_Crossfader::crossfaderActiveChanged(bool b)
{
	if(b)
	{
		ui->cb_gapless->setChecked(false);
		gaplessActiveChanged(!b);
	}

	ui->cb_crossfader->setChecked(b);
	ui->lab_crossfader->setEnabled(b);
	ui->sli_crossfader->setEnabled(b);

	SetSetting(Set::Engine_CrossFaderActive, b);
}

void GUI_Crossfader::gaplessActiveChanged(bool b)
{
	if(b)
	{
		ui->cb_crossfader->setChecked(false);
		ui->lab_crossfader->setEnabled(false);
		ui->sli_crossfader->setEnabled(false);

		crossfaderActiveChanged(!b);
	}

	ui->cb_gapless->setChecked(b);

	auto playlistMode = GetSetting(Set::PL_Mode);
	playlistMode.setGapless(b);

	SetSetting(Set::PL_Mode, playlistMode);
}

void GUI_Crossfader::engineChanged()
{
	if(!ui)
	{
		return;
	}

	const auto sink = GetSetting(Set::Engine_Sink);
	ui->cb_crossfader->setDisabled(sink == "alsa");
	ui->cb_gapless->setDisabled(sink == "alsa");

	if(sink == "alsa")
	{
		const auto crossfaderErrorString = tr("Crossfader does not work with Alsa");
		ui->cb_crossfader->setToolTip(crossfaderErrorString);
		ui->sli_crossfader->setToolTip(crossfaderErrorString);
		ui->cb_gapless->setToolTip(tr("Gapless playback does not work with Alsa"));
	}

	else
	{
		ui->cb_crossfader->setToolTip(QString());
		ui->sli_crossfader->setToolTip(QString());
		ui->cb_gapless->setToolTip(QString());
	}
}
