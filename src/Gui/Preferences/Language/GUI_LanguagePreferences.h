/* GUI_LanguagePreferences.h */

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

#ifndef GUI_LANGUAGE_PREFERENCES_H
#define GUI_LANGUAGE_PREFERENCES_H

#include "Gui/Preferences/PreferenceWidget.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_LanguagePreferences)

class GUI_LanguagePreferences :
		public Preferences::Base
{
	Q_OBJECT
	UI_CLASS(GUI_LanguagePreferences)
	PIMPL(GUI_LanguagePreferences)

public:
	explicit GUI_LanguagePreferences(const QString& identifier);
	virtual ~GUI_LanguagePreferences();

	bool commit() override;
	void revert() override;

	QString action_name() const override;

protected:
	void init_ui() override;
	void retranslate_ui() override;
	void skin_changed() override;

	void showEvent(QShowEvent*) override;

private:
	void renew_combo();

private slots:
	void combo_index_changed(int index);
	void btn_check_for_update_clicked();
	void update_check_finished();

	void btn_download_clicked();
	void download_finished();
};

#endif // GUI_LanguagePreferences_H
