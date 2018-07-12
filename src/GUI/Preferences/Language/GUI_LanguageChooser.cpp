/* GUI_LanguageChooser.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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
#include "GUI/Preferences/ui_GUI_LanguageChooser.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QFile>
#include <QDir>
#include <QRegExp>
#include <QLocale>

static QString language_name(QLocale::Language l, QLocale::Country c)
{
	QLocale loc(l, c);
	return Util::cvt_str_to_very_first_upper(loc.nativeLanguageName());
}

struct GUI_LanguageChooser::Private
{
	QMap<QString, QString>  map;

	Private()
	{
		map["br"] = language_name(QLocale::Portuguese, QLocale::Brazil);
		map["cs"] = language_name(QLocale::Czech, QLocale::CzechRepublic); // QString::fromUtf8("Český");
		map["de"] = language_name(QLocale::German, QLocale::Germany); // "Deutsch";
		map["en"] = language_name(QLocale::English, QLocale::UnitedStates); // "English";
		map["es"] = language_name(QLocale::Spanish, QLocale::Spain); // QString::fromUtf8("Español");
		map["fr"] = language_name(QLocale::French, QLocale::France); // "Francais";
		map["hu"] = language_name(QLocale::Hungarian, QLocale::Hungary); // "Magyar";
		map["it"] = language_name(QLocale::Italian, QLocale::Italy); // "Italiano";
		//map["ja"] = language_name(QLocale::Japanese, QLocale::Japan); // QString::fromUtf8("日本語");
		map["nl"] = language_name(QLocale::Dutch, QLocale::Netherlands); // "Nederlands";
		map["pl"] = language_name(QLocale::Polish, QLocale::Poland); // QString::fromUtf8("Polski");
		map["pt"] = language_name(QLocale::Portuguese, QLocale::Portugal); // QString::fromUtf8("Português");
		map["ro"] = language_name(QLocale::Romanian, QLocale::Romania); // QString::fromUtf8("Limba română");
		map["ru"] = language_name(QLocale::Russian, QLocale::Russia); // QString::fromUtf8("Русский");
		map["tr"] = language_name(QLocale::Turkish, QLocale::Turkey); // QString::fromUtf8("Türkçe");
		map["uk"] = language_name(QLocale::Ukrainian, QLocale::Ukraine); // QString::fromUtf8("Українська");
		map["zh_cn"] = language_name(QLocale::Chinese, QLocale::China); // QLocale::QString::fromUtf8("中文");
		//map["da"] = language_name(QLocale::Danish, QLocale::Denmark);
	}
};

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


void GUI_LanguageChooser::retranslate_ui()
{
	ui->retranslateUi(this);
	renew_combo();
}


bool GUI_LanguageChooser::commit()
{
	int cur_idx = ui->combo_lang->currentIndex();
	QString cur_language = ui->combo_lang->itemData(cur_idx).toString();

	_settings->set<Set::Player_Language>(cur_language);

	return true;
}

void GUI_LanguageChooser::revert() {}

// typically a qm file looks like sayonara_lang_lc.qm
void GUI_LanguageChooser::renew_combo()
{
	if(!is_ui_initialized()){
		return;
	}

	QString lang_setting = _settings->get<Set::Player_Language>();

	sp_log(Log::Info, this) << "Language setting = " << lang_setting;
	QDir dir(Util::share_path("translations"));

	QStringList filters;
	filters << "*.qm";
	//QStringList files = dir.entryList(filters);

	QStringList files;
	for(QString key : m->map.keys())
	{
		files << QString("sayonara_lang_%1.qm").arg(key);
	}

	ui->combo_lang->clear();

	int i=0;
	for(const QString& file : files)
	{
		QRegExp re(".*lang_(.*)\\.qm");
		re.setMinimal(true);

		QString country_code;
		if(re.indexIn(file) >= 0){
			country_code = re.cap(1).toLower();
		}

		else{
			continue;
		}

		if(country_code.compare("mx") == 0){
			continue;
		}

		QString icon_path = Util::share_path(
					"translations/icons/" + country_code + ".png"
		);

		QString language_name = m->map.value(country_code);

		if(language_name.size() > 0){
			ui->combo_lang->addItem(QIcon(icon_path), language_name, file);
		}

		else{
			ui->combo_lang->addItem(file, file);
		}

		if(file.contains(lang_setting, Qt::CaseInsensitive)){
			ui->combo_lang->setCurrentIndex(i);
		}

		i++;
	}
}



void GUI_LanguageChooser::init_ui()
{
	setup_parent(this, &ui);


}

void GUI_LanguageChooser::showEvent(QShowEvent* e)
{
	Base::showEvent(e);

	renew_combo();
}

QString GUI_LanguageChooser::action_name() const
{
	return tr("Language") + QString(" (Language)");
}

