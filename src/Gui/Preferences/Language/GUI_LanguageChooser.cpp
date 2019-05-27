/* GUI_LanguageChooser.cpp */

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

#include "GUI_LanguageChooser.h"
#include "Gui/Preferences/ui_GUI_LanguageChooser.h"

#include "Utils/LanguageUtils.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language.h"
#include "Gui/Utils/Style.h"


#include <QFile>
#include <QDir>
#include <QRegExp>
#include <QLocale>
#include <QStringList>

namespace Language = Util::Language;

struct GUI_LanguageChooser::Private {};

static QString get_four_letter(QComboBox* combo)
{
	return combo->currentData().toString();
}

GUI_LanguageChooser::GUI_LanguageChooser(const QString& identifier) :
	Preferences::Base(identifier)
{
	m = Pimpl::make<Private>();
}

GUI_LanguageChooser::~GUI_LanguageChooser()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

QString GUI_LanguageChooser::action_name() const
{
	return tr("Language") + QString(" (Language)");
}


void GUI_LanguageChooser::retranslate_ui()
{
	ui->retranslateUi(this);

	renew_combo();
}


void GUI_LanguageChooser::skin_changed()
{
	if(is_ui_initialized())
	{
		ui->lab_link->setText(
			Util::create_link("https://www.transifex.com/sayonara/sayonara-player", Style::is_dark())
		);
	}
}


bool GUI_LanguageChooser::commit()
{
	QString four_letter = get_four_letter(ui->combo_lang);

	SetSetting(Set::Player_Language, four_letter);

	return true;
}

void GUI_LanguageChooser::revert() {}

// typically a qm file looks like sayonara_lang_lc.qm
void GUI_LanguageChooser::renew_combo()
{
	if(!is_ui_initialized()){
		return;
	}

	ui->combo_lang->clear();

	QString language = GetSetting(Set::Player_Language);

	const QMap<QString, QLocale> locales = Lang::available_languages();

	int cur_idx = 0;
	for(auto it=locales.begin(); it != locales.end(); it++)
	{
		QString four_letter = it.key();
		QLocale loc = it.value();

		QString icon_path = Language::get_icon_path(four_letter);

		ui->combo_lang->addItem(
			QIcon(icon_path),
			Util::cvt_str_to_first_upper(loc.nativeLanguageName()),
			four_letter
		);

		if(four_letter == language){
			cur_idx = std::distance(locales.begin(), it);
		}
	}

	ui->combo_lang->setCurrentIndex(cur_idx);
}


void GUI_LanguageChooser::init_ui()
{
	setup_parent(this, &ui);

	ui->btn_download->setVisible(false);

	connect(ui->combo_lang, combo_current_index_changed_int, this, &GUI_LanguageChooser::combo_index_changed);
	connect(ui->btn_check_for_update, &QPushButton::clicked, this, &GUI_LanguageChooser::btn_check_for_update_clicked);
	connect(ui->btn_download, &QPushButton::clicked, this, &GUI_LanguageChooser::btn_download_clicked);
}


void GUI_LanguageChooser::combo_index_changed(int idx)
{
	Q_UNUSED(idx)

	QString four_letter = get_four_letter(ui->combo_lang);

	ui->btn_check_for_update->setVisible(true);
	ui->btn_download->setVisible(false);
	ui->btn_check_for_update->setEnabled(true);
	ui->lab_update_info->setText(QString());

	if(four_letter.compare("en_us", Qt::CaseInsensitive) == 0){
		ui->btn_check_for_update->setEnabled(false);
	}
}


void GUI_LanguageChooser::btn_check_for_update_clicked()
{
	ui->btn_check_for_update->setEnabled(false);

	AsyncWebAccess* awa = new AsyncWebAccess(this);
	QString url = Util::Language::get_checksum_http_path();

	connect(awa, &AsyncWebAccess::sig_finished, this, &GUI_LanguageChooser::update_check_finished);
	awa->run(url);

}

void GUI_LanguageChooser::update_check_finished()
{
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
	QString data = QString::fromUtf8(awa->data());
	bool has_error = awa->has_error();

	ui->btn_check_for_update->setEnabled(true);

	awa->deleteLater();

	if(has_error || data.isEmpty())
	{
		ui->lab_update_info->setText(tr("Cannot check for language update"));
		sp_log(Log::Warning, this) << "Cannot download checksums " << awa->url();
		return;
	}

	QStringList lines = data.split("\n");

	QString four_letter = get_four_letter(ui->combo_lang);
	QString current_checksum = Language::get_checksum(four_letter);

	for(const QString& line : lines)
	{
		if(!line.contains(four_letter)){
			continue;
		}

		QStringList splitted = line.split(" ");
		QString checksum = splitted[0];

		ui->btn_download->setVisible(current_checksum != checksum || true);
		if(current_checksum != checksum)
		{
			sp_log(Log::Info, this) << "Language update available";
			ui->lab_update_info->setText(tr("Language update available"));
		}

		else {
			sp_log(Log::Info, this) << "No need to update language";
			ui->lab_update_info->setText(tr("Language is up to date"));
		}

		break;
	}
}

void GUI_LanguageChooser::btn_download_clicked()
{
	ui->btn_check_for_update->setEnabled(false);
	ui->btn_download->setVisible(false);

	QString four_letter = get_four_letter(ui->combo_lang);
	QString url = Language::get_http_path(four_letter);

	AsyncWebAccess* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sig_finished, this, &GUI_LanguageChooser::download_finished);

	awa->run(url);
}

#include "Utils/Settings/Settings.h"

void GUI_LanguageChooser::download_finished()
{
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
	QByteArray data	= awa->data();
	bool has_error = awa->has_error();

	awa->deleteLater();
	ui->btn_check_for_update->setEnabled(true);

	if(has_error || data.isEmpty()){
		sp_log(Log::Warning, this) << "Cannot download file from " << awa->url();
		ui->lab_update_info->setText(tr("Cannot fetch language update"));
		return;
	}

	QString four_letter = get_four_letter(ui->combo_lang);
	QString filepath = Language::get_home_target_path(four_letter);
	QFile f(filepath);

	f.open(QFile::WriteOnly);
	bool b = f.write(data);
	f.close();

	if(b)
	{
		ui->lab_update_info->setText(tr("Language was updated successfully") + ".");
		sp_log(Log::Info, this) << "Language file written to " << filepath;

		Util::Language::update_language_version(four_letter);


		Settings::instance()->shout<Set::Player_Language>();
	}

	else
	{
		ui->lab_update_info->setText(tr("Cannot fetch language update"));
		sp_log(Log::Warning, this) << "Could not write language file to " << filepath;
	}
}


void GUI_LanguageChooser::showEvent(QShowEvent* e)
{
	Base::showEvent(e);

	renew_combo();
}
