/* GUI_Speed.cpp */

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

#include "GUI_Speed.h"
#include "GUI_SpeedPreferences.h"
#include "Gui/Plugins/ui_GUI_Speed.h"
#include "Gui/Utils/EventFilter.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

#include <QToolTip>
#include <QCursor>

namespace
{
	constexpr const auto StandardAFrequency = 440;
	constexpr const auto FrequencyScalingFactor = 10;
	constexpr const auto SpeedScalingFactor = 100.0f;

	void tryRestoreLastTab(int lastTab, QTabWidget* tabWidget)
	{
		lastTab = std::max(0, std::min(lastTab, tabWidget->count() - 1));
		tabWidget->setCurrentIndex(lastTab);
	}
}

GUI_Speed::GUI_Speed(QWidget* parent) :
	PlayerPlugin::Base(parent) {}

GUI_Speed::~GUI_Speed()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

void GUI_Speed::retranslate()
{
	ui->retranslateUi(this);
	ui->cbActive->setText(Lang::get(Lang::Active));
}

void GUI_Speed::initUi()
{
	setupParent(this, &ui);

	const auto active = GetSetting(Set::Engine_SpeedActive);
	const auto fSpeed = static_cast<double>(GetSetting(Set::Engine_Speed));
	const auto speed = static_cast<int>(fSpeed * SpeedScalingFactor);
	const auto pitch = GetSetting(Set::Engine_Pitch);

	activeChanged(active);
	ui->cbActive->setChecked(active);

	ui->sliSpeed->setValue(speed);
	ui->sliSpeed->setMouseTracking(true);
	ui->btnSpeed->setText(QString::number(fSpeed, 'f', 2));

	ui->cbPreservePitch->setChecked(GetSetting(Set::Engine_PreservePitch));

	ui->sliPitch->setValue(pitch * FrequencyScalingFactor);
	ui->sliPitch->setMouseTracking(true);
	ui->btnPitch->setText(QString("%1 Hz").arg(pitch));

	setupMouseEventFilters();
	tryRestoreLastTab(GetSetting(Set::Speed_LastTab), ui->tabWidget);
	ui->btnPreferences->setVisible(ui->tabWidget->currentIndex() == 0);

	connect(ui->sliSpeed, &QSlider::valueChanged, this, &GUI_Speed::speedChanged);
	connect(ui->cbActive, &QCheckBox::toggled, this, &GUI_Speed::activeToggled);
	connect(ui->cbPreservePitch, &QCheckBox::toggled, this, &GUI_Speed::preservePitchChanged);
	connect(ui->sliPitch, &QSlider::valueChanged, this, &GUI_Speed::pitchChanged);
	connect(ui->btnSpeed, &QPushButton::clicked, this, &GUI_Speed::revertSpeedClicked);
	connect(ui->btnSpeedDown, &QPushButton::clicked, this, &GUI_Speed::decreaseSpeedSlider);
	connect(ui->btnSpeedUp, &QPushButton::clicked, this, &GUI_Speed::increaseSpeedSlider);
	connect(ui->btnPitch, &QPushButton::clicked, this, &GUI_Speed::revertPitchClicked);
	connect(ui->tabWidget, &QTabWidget::currentChanged, this, &GUI_Speed::currentTabChanged);

	connect(ui->sliSpeed, &Gui::Slider::sigSliderHovered, this, &GUI_Speed::speedHovered);
	connect(ui->sliPitch, &Gui::Slider::sigSliderHovered, this, &GUI_Speed::pitchHovered);

	connect(ui->btnPreferences, &QPushButton::clicked, this, &GUI_Speed::preferencesClicked);

	ListenSetting(SetNoDB::Pitch_found, GUI_Speed::pitchFoundChanged);
	ListenSetting(Set::Speed_Step, GUI_Speed::preferencesChanged);
	ListenSetting(Set::Speed_ShowSteps, GUI_Speed::preferencesChanged);
	ListenSetting(Set::Speed_MinValue, GUI_Speed::minMaxValueChanged);
	ListenSetting(Set::Speed_MaxValue, GUI_Speed::minMaxValueChanged);
}

void GUI_Speed::setupMouseEventFilters()
{
	using Gui::MouseEnterFilter;
	using Gui::MouseLeaveFilter;

	auto* mouseEnterPitch = new MouseEnterFilter(ui->btnPitch);
	auto* mouseLeavePitch = new MouseLeaveFilter(ui->btnPitch);

	ui->btnPitch->installEventFilter(mouseEnterPitch);
	ui->btnPitch->installEventFilter(mouseLeavePitch);

	connect(mouseEnterPitch, &MouseEnterFilter::sigMouseEntered, this, [ui = this->ui]() {
		ui->btnPitch->setText(QString("%1 Hz").arg(StandardAFrequency));
	});

	connect(mouseLeavePitch, &MouseLeaveFilter::sigMouseLeft, this, [ui = this->ui]() {
		ui->btnPitch->setText(QString("%1 Hz").arg(
			ui->sliPitch->value() / FrequencyScalingFactor)); // NOLINT(readability-magic-numbers)
	});

	auto* mouseEnterSpeed = new MouseEnterFilter(ui->btnSpeed);
	auto* mouseLeaveSpeed = new MouseLeaveFilter(ui->btnSpeed);

	ui->btnSpeed->installEventFilter(mouseEnterSpeed);
	ui->btnSpeed->installEventFilter(mouseLeaveSpeed);

	connect(mouseEnterSpeed, &MouseEnterFilter::sigMouseEntered, this, [ui = this->ui]() {
		ui->btnSpeed->setText(QString::number(1.0, 'f', 2));
	});

	connect(mouseLeaveSpeed, &MouseLeaveFilter::sigMouseLeft, this, [ui = this->ui]() {
		ui->btnSpeed->setText(QString::number(ui->sliSpeed->value() / SpeedScalingFactor, 'f', 2));
	});
}

QString GUI_Speed::name() const { return "Speed"; }

QString GUI_Speed::displayName() const
{
	return tr("%1 and %2")
		.arg(tr("Speed"))
		.arg(tr("Pitch"));
}

void GUI_Speed::speedChanged(const int val)
{
	ui->btnSpeed->setText(QString::number(val / SpeedScalingFactor, 'f', 2));
	SetSetting(Set::Engine_Speed,
	           ui->sliSpeed->value() / SpeedScalingFactor); // NOLINT(readability-uppercase-literal-suffix)
}

void GUI_Speed::activeChanged(const bool active)
{
	ui->sliSpeed->setEnabled(active);
	ui->btnSpeed->setEnabled(active);
	ui->sliPitch->setEnabled(active);
	ui->cbPreservePitch->setEnabled(active);
	ui->btnPitch->setEnabled(active);
	ui->btnSpeedDown->setEnabled(active);
	ui->btnSpeedUp->setEnabled(active);
}

void GUI_Speed::activeToggled(const bool active)
{
	activeChanged(active);
	SetSetting(Set::Engine_SpeedActive, active);
}

void GUI_Speed::preservePitchChanged(const bool enabled) // NOLINT(readability-convert-member-functions-to-static)
{
	SetSetting(Set::Engine_PreservePitch, enabled);
}

void GUI_Speed::pitchChanged(const int sliderValue)
{
	const auto pitch = sliderValue / FrequencyScalingFactor;
	SetSetting(Set::Engine_Pitch, pitch);
	ui->btnPitch->setText(QString("%1 Hz").arg(pitch));
}

void GUI_Speed::revertSpeedClicked()
{
	ui->sliSpeed->setValue(100); // NOLINT(readability-magic-numbers)
}

void GUI_Speed::revertPitchClicked()
{
	ui->sliPitch->setValue(StandardAFrequency * FrequencyScalingFactor);
}

void GUI_Speed::pitchHovered(const int val) // NOLINT(readability-convert-member-functions-to-static)
{
	QToolTip::showText(QCursor::pos(), QString::number(val / FrequencyScalingFactor));
}

void GUI_Speed::speedHovered(const int val) // NOLINT(readability-convert-member-functions-to-static)
{
	QToolTip::showText(QCursor::pos(), QString::number(val / SpeedScalingFactor));
}

void GUI_Speed::currentTabChanged(const int idx) // NOLINT(readability-convert-member-functions-to-static)
{
	ui->btnPreferences->setVisible(idx == 0);
	SetSetting(Set::Speed_LastTab, idx);
}

void GUI_Speed::pitchFoundChanged()
{
	const auto pitchFound = GetSetting(SetNoDB::Pitch_found);
	if(!pitchFound)
	{
		ui->cbActive->setChecked(false);
		activeChanged(false);
		ui->cbActive->setToolTip(tr("%1 not found").arg("gstreamer bad plugins") + "<br />" +
		                         tr("%1 not found").arg("libsoundtouch"));
	}

	else
	{
		ui->cbActive->setToolTip("");
	}

	ui->cbActive->setEnabled(pitchFound);
}

void GUI_Speed::preferencesClicked()
{
	auto* preferences = new GUI_SpeedPreferences(this);
	connect(preferences, &QDialog::finished, this, [preferences](const auto /*result*/) {
		preferences->deleteLater();
	});

	preferences->show();
}

void GUI_Speed::preferencesChanged()
{
	ui->btnSpeedDown->setVisible(GetSetting(Set::Speed_ShowSteps));
	ui->btnSpeedUp->setVisible(GetSetting(Set::Speed_ShowSteps));

	ui->btnSpeedDown->setText(QString("-%1%").arg(GetSetting(Set::Speed_Step)));
	ui->btnSpeedUp->setText(QString("+%1%").arg(GetSetting(Set::Speed_Step)));
}

void GUI_Speed::minMaxValueChanged()
{
	const auto originalValue = ui->sliSpeed->value();
	auto currentValue = originalValue;
	currentValue = std::min(currentValue, GetSetting(Set::Speed_MaxValue));
	currentValue = std::max(currentValue, GetSetting(Set::Speed_MinValue));

	if(currentValue != originalValue)
	{
		ui->sliSpeed->setValue(currentValue);
		SetSetting(Set::Engine_Speed, currentValue / SpeedScalingFactor);
	}

	ui->sliSpeed->setMinimum(GetSetting(Set::Speed_MinValue));
	ui->sliSpeed->setMaximum(GetSetting(Set::Speed_MaxValue));
}

void GUI_Speed::increaseSpeedSlider()
{
	ui->sliSpeed->setValue(std::min(ui->sliSpeed->maximum(), ui->sliSpeed->value() + GetSetting(Set::Speed_Step)));
}

void GUI_Speed::decreaseSpeedSlider()
{
	ui->sliSpeed->setValue(std::max(ui->sliSpeed->minimum(), ui->sliSpeed->value() - GetSetting(Set::Speed_Step)));
}
