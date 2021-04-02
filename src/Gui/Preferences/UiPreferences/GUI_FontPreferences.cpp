
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

#include "GUI_FontPreferences.h"
#include "Gui/Preferences/ui_GUI_FontPreferences.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

#include <QRegExp>

namespace
{
	float convertComboTextToValue(const QString& text)
	{
		auto re = QRegExp("([0-9]+)%$");
		auto index = re.indexIn(text);
		return (index < 0)
		       ? 1.0
		       : re.cap(1).toFloat() / 100.0;
	}

	QString convertValueToComboText(float value)
	{
		return (value < 0.5 || value > 2.0)
		       ? "100%"
		       : QString("%1%").arg(static_cast<int>(value * 100));
	}
}

struct GUI_FontPreferences::Private
{
};

GUI_FontPreferences::GUI_FontPreferences(QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>();
}

GUI_FontPreferences::~GUI_FontPreferences()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

void GUI_FontPreferences::initUi()
{
	if(ui)
	{
		return;
	}

	ui = new Ui::GUI_FontPreferences();
	ui->setupUi(this);

	revert();
}

QString GUI_FontPreferences::actionName() const
{
	return Lang::get(Lang::Fonts);
}

bool GUI_FontPreferences::commit()
{
	if(!ui)
	{
		return true;
	}

	SetSetting(Set::Player_ScalingFactor, convertComboTextToValue(ui->comboScalingFactor->currentText()));
	SetSetting(Set::Lib_FontBold, ui->cbBold->isChecked());

	return true;
}

void GUI_FontPreferences::revert()
{
	if(!ui)
	{
		return;
	}

	auto scalingText = convertValueToComboText(GetSetting(Set::Player_ScalingFactor));
	ui->comboScalingFactor->setCurrentText(scalingText);
	ui->cbBold->setChecked(GetSetting(Set::Lib_FontBold));
}

void GUI_FontPreferences::languageChanged()
{
	if(!ui)
	{
		return;
	}

	ui->retranslateUi(this);
}

void GUI_FontPreferences::showEvent(QShowEvent* e)
{
	if(!ui)
	{
		initUi();
	}

	Gui::Widget::showEvent(e);
}
