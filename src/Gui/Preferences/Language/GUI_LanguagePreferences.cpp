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
#include "Gui/Utils/Style.h"

#include <QFile>
#include <QDir>
#include <QRegExp>
#include <QLocale>
#include <QStringList>

namespace Language = Util::Language;

struct GUI_LanguagePreferences::Private {};

static QString getFourLetter(QComboBox* combo)
{
	return combo->currentData().toString();
}

GUI_LanguagePreferences::GUI_LanguagePreferences(const QString& identifier) :
	Preferences::Base(identifier)
{
	m = Pimpl::make<Private>();
}

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
	QString fourLetter = getFourLetter(ui->comboLanguages);

	SetSetting(Set::Player_Language, fourLetter);

	return true;
}

void GUI_LanguagePreferences::revert() {}

// typically a qm file looks like sayonara_lang_lc.qm
void GUI_LanguagePreferences::refreshCombobox()
{
	if(!isUiInitialized()){
		return;
	}

	ui->comboLanguages->clear();

	QString language = GetSetting(Set::Player_Language);

	const QMap<QString, QLocale> locales = Lang::availableLanguages();

	int cur_idx = 0;
	for(auto it=locales.begin(); it != locales.end(); it++)
	{
		QString fourLetter = it.key();
		QString icon_path = Language::getIconPath(fourLetter);

		QLocale loc = it.value();
		QString language_name = Util::stringToFirstUpper(loc.nativeLanguageName());
		if(fourLetter.startsWith("en")){
			language_name = "English";
		}

		ui->comboLanguages->addItem
		(
			QIcon(icon_path),
			language_name,
			fourLetter
		);

		if(fourLetter == language){
			cur_idx = std::distance(locales.begin(), it);
		}
	}

	ui->comboLanguages->setCurrentIndex(cur_idx);
}


void GUI_LanguagePreferences::initUi()
{
	setupParent(this, &ui);

	ui->btnDownload->setVisible(false);

	connect(ui->comboLanguages, combo_current_index_changed_int, this, &GUI_LanguagePreferences::currentIndexChanged);
	connect(ui->btnCheckForUpdate, &QPushButton::clicked, this, &GUI_LanguagePreferences::checkForUpdateClicked);
	connect(ui->btnDownload, &QPushButton::clicked, this, &GUI_LanguagePreferences::downloadClicked);
}


void GUI_LanguagePreferences::currentIndexChanged(int idx)
{
	Q_UNUSED(idx)

	QString fourLetter = getFourLetter(ui->comboLanguages);

	ui->btnCheckForUpdate->setVisible(true);
	ui->btnDownload->setVisible(false);
	ui->btnCheckForUpdate->setEnabled(true);
	ui->labUpdateInfo->setText(QString());
}


void GUI_LanguagePreferences::checkForUpdateClicked()
{
	ui->btnCheckForUpdate->setEnabled(false);

	AsyncWebAccess* awa = new AsyncWebAccess(this);
	QString url = Util::Language::getChecksumHttpPath();

	connect(awa, &AsyncWebAccess::sigFinished, this, &GUI_LanguagePreferences::updateCheckFinished);
	awa->run(url);

}

void GUI_LanguagePreferences::updateCheckFinished()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	QString data = QString::fromUtf8(awa->data());
	bool has_error = awa->hasError();

	ui->btnCheckForUpdate->setVisible(false);
	ui->btnCheckForUpdate->setEnabled(true);

	awa->deleteLater();

	if(has_error || data.isEmpty())
	{
		ui->labUpdateInfo->setText(tr("Cannot check for language update"));

		spLog(Log::Warning, this) << "Cannot download checksums " << awa->url();
		return;
	}

	QStringList lines = data.split("\n");

	QString fourLetter = getFourLetter(ui->comboLanguages);
	QString current_checksum = Language::getChecksum(fourLetter);

	bool download_enabled = false;
	for(const QString& line : lines)
	{
		if(!line.contains(fourLetter)){
			continue;
		}

		QStringList splitted = line.split(" ");
		QString checksum = splitted[0];

		download_enabled = (current_checksum != checksum);

		if(current_checksum != checksum)
		{
			spLog(Log::Info, this) << "Language update available";
			ui->labUpdateInfo->setText(tr("Language update available"));
		}

		else {
			spLog(Log::Info, this) << "No need to update language";
			ui->labUpdateInfo->setText(tr("Language is up to date"));
		}

		break;
	}

	ui->btnDownload->setVisible(download_enabled);
	ui->btnDownload->setEnabled(download_enabled);
	ui->btnCheckForUpdate->setVisible(!download_enabled);
}

void GUI_LanguagePreferences::downloadClicked()
{
	ui->btnDownload->setEnabled(false);

	QString fourLetter = getFourLetter(ui->comboLanguages);
	QString url = Language::getHttpPath(fourLetter);

	AsyncWebAccess* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sigFinished, this, &GUI_LanguagePreferences::downloadFinished);

	awa->run(url);
}


void GUI_LanguagePreferences::downloadFinished()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	QByteArray data	= awa->data();
	bool has_error = awa->hasError();

	awa->deleteLater();

	ui->btnCheckForUpdate->setVisible(true);

	ui->btnDownload->setEnabled(true);
	ui->btnDownload->setVisible(false);

	if(has_error || data.isEmpty())
	{
		spLog(Log::Warning, this) << "Cannot download file from " << awa->url();
		ui->labUpdateInfo->setText(tr("Cannot fetch language update"));
		return;
	}

	QString fourLetter = getFourLetter(ui->comboLanguages);
	QString filepath = Language::getHomeTargetPath(fourLetter);
	QFile f(filepath);

	f.open(QFile::WriteOnly);
	bool b = f.write(data);
	f.close();

	if(b)
	{
		ui->labUpdateInfo->setText(tr("Language was updated successfully") + ".");
		spLog(Log::Info, this) << "Language file written to " << filepath;

		Util::Language::updateLanguageVersion(fourLetter);

		Settings::instance()->shout<Set::Player_Language>();
	}

	else
	{
		ui->labUpdateInfo->setText(tr("Cannot fetch language update"));
		spLog(Log::Warning, this) << "Could not write language file to " << filepath;
	}
}


void GUI_LanguagePreferences::showEvent(QShowEvent* e)
{
	Base::showEvent(e);

	refreshCombobox();
}
