/* GUI_Speed.cpp */

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

#include "GUI_Speed.h"
#include "Gui/ui_GUI_Speed.h"
#include "Gui/Utils/EventFilter.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

#include <QToolTip>
#include <QCursor>

GUI_Speed::GUI_Speed(QWidget *parent) :
	PlayerPlugin::Base(parent) {}

GUI_Speed::~GUI_Speed()
{
	if(ui){
		delete ui; ui=nullptr;
	}
}

void GUI_Speed::retranslate_ui()
{
	ui->retranslateUi(this);
	ui->cb_active->setText(Lang::get(Lang::Active));
}

void GUI_Speed::init_ui()
{
	setup_parent(this, &ui);

	bool active = GetSetting(Set::Engine_SpeedActive);
	float speed = GetSetting(Set::Engine_Speed) * 1.0f;
	int pitch = GetSetting(Set::Engine_Pitch) * 10;

	active_changed(active);

	ui->sli_speed->setValue(speed * 100);
	ui->sli_speed->setMouseTracking(true);

	speed_changed(speed * 100);

	ui->cb_preserve_pitch->setChecked( GetSetting(Set::Engine_PreservePitch));

	ui->sli_pitch->setValue(pitch);
	ui->sli_pitch->setMouseTracking(true);
	pitch_changed(pitch);

	MouseEnterFilter* mef_pitch = new MouseEnterFilter(ui->btn_pitch);
	MouseEnterFilter* mef_speed = new MouseEnterFilter(ui->btn_speed);

	MouseLeaveFilter* mlf_pitch = new MouseLeaveFilter(ui->btn_pitch);
	MouseLeaveFilter* mlf_speed = new MouseLeaveFilter(ui->btn_speed);

	connect(mef_pitch, &MouseEnterFilter::sig_mouse_entered, this, [=](){
		ui->btn_pitch->setText("440 Hz");
	});

	connect(mef_speed, &MouseEnterFilter::sig_mouse_entered, this, [=](){
		ui->btn_speed->setText(QString::number(1.0f, 'f', 2));
	});

	connect(mlf_pitch, &MouseLeaveFilter::sig_mouse_left, this, [=](){
		ui->btn_pitch->setText(QString::number(ui->sli_pitch->value() / 10) + " Hz");
	});

	connect(mlf_speed, &MouseLeaveFilter::sig_mouse_left, this, [=](){
		ui->btn_speed->setText(QString::number(ui->sli_speed->value() / 100.0f, 'f', 2));
	});

	ui->btn_speed->installEventFilter(mef_speed);
	ui->btn_speed->installEventFilter(mlf_speed);

	ui->btn_pitch->installEventFilter(mef_pitch);
	ui->btn_pitch->installEventFilter(mlf_pitch);

	connect(ui->sli_speed, &QSlider::valueChanged, this, &GUI_Speed::speed_changed);
	connect(ui->cb_active, &QCheckBox::toggled, this, &GUI_Speed::active_changed);
	connect(ui->cb_preserve_pitch, &QCheckBox::toggled, this, &GUI_Speed::preserve_pitch_changed);
	connect(ui->sli_pitch, &QSlider::valueChanged, this, &GUI_Speed::pitch_changed);
	connect(ui->btn_speed, &QPushButton::clicked, this, &GUI_Speed::revert_speed_clicked);
	connect(ui->btn_pitch, &QPushButton::clicked, this, &GUI_Speed::revert_pitch_clicked);

	connect(ui->sli_speed, &Gui::Slider::sig_slider_hovered, this, &GUI_Speed::speed_hovered);
	connect(ui->sli_pitch, &Gui::Slider::sig_slider_hovered, this, &GUI_Speed::pitch_hovered);

	ListenSetting(SetNoDB::Pitch_found, GUI_Speed::_sl_pitch_found_changed);
}


QString GUI_Speed::get_name() const
{
	return "Speed";
}

QString GUI_Speed::get_display_name() const
{
	return tr("Speed") + "/" + tr("Pitch");
}


void GUI_Speed::speed_changed(int val)
{
	float val_f = val / 100.0f;

	ui->btn_speed->setText(QString::number(val_f, 'f', 2));
	SetSetting(Set::Engine_Speed, ui->sli_speed->value() / 100.0f);
}


void GUI_Speed::active_changed(bool active)
{
	ui->cb_active->setChecked(active);

	ui->sli_speed->setEnabled( active);
	ui->btn_speed->setEnabled(active);
	ui->sli_pitch->setEnabled(active);
	ui->cb_preserve_pitch->setEnabled(active);
	ui->btn_pitch->setEnabled(active);

	SetSetting(Set::Engine_SpeedActive, active);
}

void GUI_Speed::preserve_pitch_changed(bool enabled)
{
	SetSetting(Set::Engine_PreservePitch, enabled);
}

void GUI_Speed::pitch_changed(int pitch)
{
	pitch = pitch / 10;
	SetSetting(Set::Engine_Pitch, pitch);
	ui->btn_pitch->setText(QString::number(pitch) + " Hz");
}

void GUI_Speed::revert_speed_clicked()
{
	ui->sli_speed->setValue(100);
}

void GUI_Speed::revert_pitch_clicked()
{
	ui->sli_pitch->setValue(4400);
}

void GUI_Speed::pitch_hovered(int val)
{
	QToolTip::showText( QCursor::pos(), QString::number(val / 10));
}

void GUI_Speed::speed_hovered(int val)
{
	QToolTip::showText( QCursor::pos(), QString::number((float) (val / 100.0f)));
}

void GUI_Speed::_sl_pitch_found_changed()
{
	bool pitch_found = GetSetting(SetNoDB::Pitch_found);
	if(!pitch_found){
		ui->cb_active->setChecked(false);
		active_changed(false);
		ui->cb_active->setToolTip(tr("%1 not found").arg("gstreamer bad plugins") + "<br />" +
							  tr("%1 not found").arg("libsoundtouch"));
	}

	else{
		ui->cb_active->setToolTip("");
	}

	ui->cb_active->setEnabled(pitch_found);
}
