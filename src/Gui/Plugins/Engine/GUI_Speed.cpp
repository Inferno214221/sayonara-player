/* GUI_Speed.cpp */

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

#include "GUI_Speed.h"
#include "Gui/Plugins/ui_GUI_Speed.h"
#include "Gui/Utils/EventFilter.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

#include <QToolTip>
#include <QCursor>

GUI_Speed::GUI_Speed(QWidget* parent) :
	PlayerPlugin::Base(parent) {}

GUI_Speed::~GUI_Speed()
{
	if(ui){
		delete ui; ui=nullptr;
	}
}

void GUI_Speed::retranslate()
{
	ui->retranslateUi(this);
	ui->cbActive->setText(Lang::get(Lang::Active));
}

void GUI_Speed::initUi()
{
	using Gui::MouseEnterFilter;
	using Gui::MouseLeaveFilter;

	setupParent(this, &ui);

	bool active =	GetSetting(Set::Engine_SpeedActive);
    double fSpeed =	double(GetSetting(Set::Engine_Speed));
    int speed =		int(fSpeed * 100);
	int pitch =		GetSetting(Set::Engine_Pitch);

	activeChanged(active);
	ui->cbActive->setChecked(active);

	ui->sliSpeed->setValue(speed);
	ui->sliSpeed->setMouseTracking(true);
	ui->btnSpeed->setText(QString::number(fSpeed, 'f', 2));

	ui->cbPreservePitch->setChecked( GetSetting(Set::Engine_PreservePitch));

	ui->sliPitch->setValue(pitch * 10);
	ui->sliPitch->setMouseTracking(true);
	ui->btnPitch->setText(QString::number(pitch) + " Hz");

	{ // mouse events for speed button
		auto* mefSpeed = new MouseEnterFilter(ui->btnSpeed);
		auto* mlfSpeed = new MouseLeaveFilter(ui->btnSpeed);

		ui->btnSpeed->installEventFilter(mefSpeed);
		ui->btnSpeed->installEventFilter(mlfSpeed);

		connect(mefSpeed, &MouseEnterFilter::sigMouseEntered, this, [=](){
			ui->btnSpeed->setText(QString::number(1.0, 'f', 2));
		});

		connect(mlfSpeed, &MouseLeaveFilter::sigMouseLeft, this, [=](){
			ui->btnSpeed->setText(QString::number(ui->sliSpeed->value() / 100.0, 'f', 2));
		});
	}

	{ // mouse events for pitch button
		auto* mef_pitch = new MouseEnterFilter(ui->btnPitch);
		auto* mlf_pitch = new MouseLeaveFilter(ui->btnPitch);

		ui->btnPitch->installEventFilter(mef_pitch);
		ui->btnPitch->installEventFilter(mlf_pitch);

		connect(mef_pitch, &MouseEnterFilter::sigMouseEntered, this, [=](){
			ui->btnPitch->setText("440 Hz");
		});

		connect(mlf_pitch, &MouseLeaveFilter::sigMouseLeft, this, [=](){
			ui->btnPitch->setText(QString("%1 Hz").arg(ui->sliPitch->value() / 10));
		});

	}

	{ // restore tab
		int last_tab = GetSetting(Set::Speed_LastTab);
		last_tab = std::max(0, std::min(last_tab, ui->tabWidget->count() - 1));

		ui->tabWidget->setCurrentIndex(last_tab);
	}

	connect(ui->sliSpeed, &QSlider::valueChanged, this, &GUI_Speed::speedChanged);
	connect(ui->cbActive, &QCheckBox::toggled, this, &GUI_Speed::activeToggled);
	connect(ui->cbPreservePitch, &QCheckBox::toggled, this, &GUI_Speed::preservePitchChanged);
	connect(ui->sliPitch, &QSlider::valueChanged, this, &GUI_Speed::pitchChanged);
	connect(ui->btnSpeed, &QPushButton::clicked, this, &GUI_Speed::revertSpeedClicked);
	connect(ui->btnPitch, &QPushButton::clicked, this, &GUI_Speed::revertPitchClicked);
	connect(ui->tabWidget, &QTabWidget::currentChanged, this, &GUI_Speed::currentTabChanged);

	connect(ui->sliSpeed, &Gui::Slider::sigSliderHovered, this, &GUI_Speed::speedHovered);
	connect(ui->sliPitch, &Gui::Slider::sigSliderHovered, this, &GUI_Speed::pitchHovered);

	ListenSetting(SetNoDB::Pitch_found, GUI_Speed::pitchFoundChanged);
}


QString GUI_Speed::name() const
{
	return "Speed";
}

QString GUI_Speed::displayName() const
{
	QString s = tr("Speed");
	QString p = tr("Pitch");

	return tr("%1 and %2")
		.arg(s)
		.arg(p);
}

void GUI_Speed::speedChanged(int val)
{
	ui->btnSpeed->setText(QString::number(val / 100.0, 'f', 2));
	SetSetting(Set::Engine_Speed, ui->sliSpeed->value() / 100.0f);
}

void GUI_Speed::activeChanged(bool active)
{
	ui->sliSpeed->setEnabled( active);
	ui->btnSpeed->setEnabled(active);
	ui->sliPitch->setEnabled(active);
	ui->cbPreservePitch->setEnabled(active);
	ui->btnPitch->setEnabled(active);
}

void GUI_Speed::activeToggled(bool active)
{
	activeChanged(active);
	SetSetting(Set::Engine_SpeedActive, active);
}

void GUI_Speed::preservePitchChanged(bool enabled)
{
	SetSetting(Set::Engine_PreservePitch, enabled);
}

void GUI_Speed::pitchChanged(int slider_value)
{
	int pitch = slider_value / 10;
	SetSetting(Set::Engine_Pitch, pitch);
	ui->btnPitch->setText(QString::number(pitch) + " Hz");
}

void GUI_Speed::revertSpeedClicked()
{
	ui->sliSpeed->setValue(100);
}

void GUI_Speed::revertPitchClicked()
{
	ui->sliPitch->setValue(4400);
}

void GUI_Speed::pitchHovered(int val)
{
	QToolTip::showText( QCursor::pos(), QString::number(val / 10));
}

void GUI_Speed::speedHovered(int val)
{
	QToolTip::showText( QCursor::pos(), QString::number(val / 100.0));
}

void GUI_Speed::currentTabChanged(int idx)
{
	SetSetting(Set::Speed_LastTab, idx);
}

void GUI_Speed::pitchFoundChanged()
{
	bool pitch_found = GetSetting(SetNoDB::Pitch_found);
	if(!pitch_found)
	{
		ui->cbActive->setChecked(false);
		activeChanged(false);
		ui->cbActive->setToolTip(tr("%1 not found").arg("gstreamer bad plugins") + "<br />" +
							  tr("%1 not found").arg("libsoundtouch"));
	}

	else{
		ui->cbActive->setToolTip("");
	}

	ui->cbActive->setEnabled(pitch_found);
}
