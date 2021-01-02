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

#include "Utils/Utils.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Language/LanguageUtils.h"
#include "Utils/Message/Message.h"

#include "Gui/Utils/Style.h"

#include <QFile>
#include <QDir>
#include <QRegExp>
#include <QLocale>
#include <QStringList>
#include <QFileDialog>

namespace Language = Util::Language;

static QString getLanguageCode(QComboBox* combo)
{
	return combo->currentData().toString();
}

GUI_LanguagePreferences::GUI_LanguagePreferences(const QString& identifier) :
	Preferences::Base(identifier)
{}

GUI_LanguagePreferences::~GUI_LanguagePreferences()
{
	if(ui)
	{
		delete ui; ui=nullptr;
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
	const QString languageCode = getLanguageCode(ui->comboLanguages);
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
	if(!isUiInitialized()){
		return;
	}

	ui->comboLanguages->clear();

	const QString playerLanguage = GetSetting(Set::Player_Language);
	const QMap<QString, QLocale> locales = Util::Language::availableLanguages();

	int englishIndex = -1;
	int currentIndex = -1;
	int i = 0;
	for(auto it=locales.begin(); it != locales.end(); it++, i++)
	{
		const QString languageCode = it.key();
		const QString iconPath = Language::getIconPath(languageCode);

		const QLocale locale = it.value();

		QString languageName = Util::stringToVeryFirstUpper(locale.nativeLanguageName().toCaseFolded());
		if(languageCode.startsWith("en", Qt::CaseInsensitive)){
			languageName = "English";
			englishIndex = i;
		}

		ui->comboLanguages->addItem
		(
			QIcon(iconPath),
			languageName,
			languageCode
		);

		if(languageCode.compare(playerLanguage, Qt::CaseInsensitive) == 0){
			currentIndex = i;
		}
	}

	if(currentIndex < 0) {
		currentIndex = englishIndex;
	}

	if(currentIndex < 0) {
		ui->comboLanguages->setCurrentIndex(0);
	}

	else {
		ui->comboLanguages->setCurrentIndex(currentIndex);
	}
}

void GUI_LanguagePreferences::initUi()
{
	setupParent(this, &ui);

	connect(ui->btnCheckForUpdate, &QPushButton::clicked, this, &GUI_LanguagePreferences::checkForUpdateClicked);
	connect(ui->btnImport, &QPushButton::clicked, this, &GUI_LanguagePreferences::importLanguageClicked);
}

void GUI_LanguagePreferences::checkForUpdateClicked()
{
	auto* awa = new AsyncWebAccess(this);
	const QString url = Util::Language::getChecksumHttpPath();

	connect(awa, &AsyncWebAccess::sigFinished, this, &GUI_LanguagePreferences::updateCheckFinished);
	awa->run(url);
}

void GUI_LanguagePreferences::updateCheckFinished()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	const QString data = QString::fromUtf8(awa->data());
	bool hasError = awa->hasError();

	awa->deleteLater();

	if(hasError || data.isEmpty())
	{
		Message::warning(tr("Cannot check for language update"));
		spLog(Log::Warning, this) << "Cannot download checksums " << awa->url();
		return;
	}

	const QStringList lines = data.split("\n");
	const QString currentLanguageCode = getLanguageCode(ui->comboLanguages);
	const QString currentChecksum = Language::getChecksum(currentLanguageCode);

	bool isUpdateAvailable = false;
	for(const QString& line : lines)
	{
		const QStringList splitted = line.split(QRegExp("\\s+"));
		if(splitted.size() != 2){
			continue;
		}

		const QString languageCode = Language::extractLanguageCode(splitted[1]);
		if(languageCode == currentLanguageCode)
		{
			const QString checksum = splitted[0];
			isUpdateAvailable = (currentChecksum != checksum);

			break;
		}
	}

	if(!isUpdateAvailable)
	{
		spLog(Log::Info, this) << "No need to update language";
		Message::info(tr("Language is up to date"));
		return;
	}

	spLog(Log::Info, this) << "Language update available";
	downloadUpdate(currentLanguageCode);
}

void GUI_LanguagePreferences::downloadUpdate(const QString& languageCode)
{
	const QString url = Language::getHttpPath(languageCode);

	auto* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sigFinished, this, &GUI_LanguagePreferences::downloadFinished);
	awa->run(url);
}

void GUI_LanguagePreferences::downloadFinished()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	QByteArray data	= awa->data();
	bool hasError = awa->hasError();

	awa->deleteLater();

	if(hasError || data.isEmpty())
	{
		spLog(Log::Warning, this) << "Cannot download file from " << awa->url();
		Message::warning(tr("Cannot fetch language update"));
		return;
	}

	const QString languageCode = getLanguageCode(ui->comboLanguages);
	const QString filepath = Language::getHomeTargetPath(languageCode);

	QFile f(filepath);
	f.open(QFile::WriteOnly);
	bool b = f.write(data);
	f.close();

	if(b)
	{
		Message::info(tr("Language was updated successfully") + ".");
		spLog(Log::Info, this) << "Language file written to " << filepath;

		Util::Language::updateLanguageVersion(languageCode);

		Settings::instance()->shout<Set::Player_Language>();
	}

	else
	{
		Message::warning(tr("Cannot fetch language update"));
		spLog(Log::Warning, this) << "Could not write language file to " << filepath;
	}
}

void GUI_LanguagePreferences::importLanguageClicked()
{
	const QString filename = QFileDialog::getOpenFileName
	(
		this,
		Lang::get(Lang::ImportFiles),
		QDir::homePath(),
		"*.qm"
	);

	if(filename.isEmpty()) {
		return;
	}

	bool success = Language::importLanguageFile(filename);
	if(!success)
	{
		Message::warning(tr("The language file could not be imported"));
		return;
	}

	const QString& newLanguageCode = Language::extractLanguageCode(filename);
	Message::info(tr("The language file was imported successfully"));

	refreshCombobox();

	int index = ui->comboLanguages->findData(newLanguageCode);
	if(index >= 0) {
		ui->comboLanguages->setCurrentIndex(index);
	}
}

void GUI_LanguagePreferences::showEvent(QShowEvent* e)
{
	Base::showEvent(e);

	refreshCombobox();
}
