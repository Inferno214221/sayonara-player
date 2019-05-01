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
#include "GUI/Preferences/ui_GUI_LanguageChooser.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language.h"
#include "GUI/Utils/Style.h"

#include <QFile>
#include <QDir>
#include <QRegExp>
#include <QLocale>

struct GUI_LanguageChooser::Private {};

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
	int cur_idx = ui->combo_lang->currentIndex();
	QString four_letter = ui->combo_lang->itemData(cur_idx).toString();

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

		QString rel_icon_path = QString("translations/icons/%1.png").arg(four_letter);
		QString abs_icon_path = Util::share_path(rel_icon_path);

		ui->combo_lang->addItem(
			QIcon(abs_icon_path),
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

