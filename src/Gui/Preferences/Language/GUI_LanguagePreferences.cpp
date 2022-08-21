/* GUI_LanguagePreferences.cpp */

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

#include "GUI_LanguagePreferences.h"

#include "Gui/Preferences/ui_GUI_LanguagePreferences.h"

#include "Components/Preferences/LanguagePreferences.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"

#include "Gui/Utils/Style.h"

#include <QDir>
#include <QFileDialog>

namespace
{
	QString getLanguageCode(QComboBox* combo)
	{
		return combo->currentData().toString();
	}
}

struct GUI_LanguagePreferences::Private
{
	LanguagePreferences* languagePreferences;

	explicit Private(QObject* parent) :
		languagePreferences {new LanguagePreferences(parent)} {}
};

GUI_LanguagePreferences::GUI_LanguagePreferences(const QString& identifier) :
	Preferences::Base(identifier)
{
	m = Pimpl::make<Private>(this);

	connect(m->languagePreferences, &LanguagePreferences::sigInfo, this, [](const QString& info) {
		Message::info(info);
	});

	connect(m->languagePreferences, &LanguagePreferences::sigWarning, this, [](const QString& warning) {
		Message::warning(warning);
	});
}

GUI_LanguagePreferences::~GUI_LanguagePreferences()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

QString GUI_LanguagePreferences::actionName() const
{
	return tr("Language") + QString(" (Language)");
}

void GUI_LanguagePreferences::retranslate()
{
	ui->retranslateUi(this);
	refreshCombobox();
}

void GUI_LanguagePreferences::skinChanged()
{
	if(isUiInitialized())
	{
		ui->labLink->setText(
			Util::createLink("https://www.transifex.com/sayonara/sayonara-player", Style::isDark())
		);
	}
}

bool GUI_LanguagePreferences::commit()
{
	const auto languageCode = getLanguageCode(ui->comboLanguages);
	SetSetting(Set::Player_Language, languageCode);

	return true;
}

void GUI_LanguagePreferences::revert()
{
	refreshCombobox();
}

// typically a qm file looks like sayonara_lang_lc.qm
void GUI_LanguagePreferences::refreshCombobox()
{
	if(!isUiInitialized())
	{
		return;
	}

	ui->comboLanguages->blockSignals(true);
	ui->comboLanguages->clear();

	const auto [languageItems, currentLanguageIndex] = LanguagePreferences::getAllLanguages();
	for(const auto& item: languageItems)
	{
		ui->comboLanguages->addItem(
			QIcon(item.iconPath),
			item.languageName,
			item.languageCode);
	}

	ui->comboLanguages->setCurrentIndex(currentLanguageIndex);
	ui->comboLanguages->blockSignals(false);
}

void GUI_LanguagePreferences::initUi()
{
	setupParent(this, &ui);

	connect(ui->btnCheckForUpdate, &QPushButton::clicked, this, &GUI_LanguagePreferences::checkForUpdateClicked);
	connect(ui->btnImport, &QPushButton::clicked, this, &GUI_LanguagePreferences::importLanguageClicked);
}

void GUI_LanguagePreferences::checkForUpdateClicked()
{
	const auto languageCode = getLanguageCode(ui->comboLanguages);
	m->languagePreferences->checkForUpdate(languageCode);
}

void GUI_LanguagePreferences::importLanguageClicked()
{
	const auto filename = QFileDialog::getOpenFileName(
		this,
		Lang::get(Lang::ImportFiles),
		QDir::homePath(),
		"*.qm");

	const auto newLanguageCode = m->languagePreferences->importLanguage(filename);
	if(!newLanguageCode.isEmpty())
	{
		refreshCombobox();

		const auto index = ui->comboLanguages->findData(newLanguageCode);
		if(index >= 0)
		{
			ui->comboLanguages->setCurrentIndex(index);
		}
	}
}

void GUI_LanguagePreferences::showEvent(QShowEvent* e)
{
	Base::showEvent(e);
	refreshCombobox();
}
