/* GUI_LanguageChooser.h */

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

#ifndef GUI_LANGUAGECHOOSER_H
#define GUI_LANGUAGECHOOSER_H

#include "Interfaces/PreferenceDialog/PreferenceWidget.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_LanguageChooser)

class GUI_LanguageChooser :
		public Preferences::Base
{
	Q_OBJECT
	UI_CLASS(GUI_LanguageChooser)
	PIMPL(GUI_LanguageChooser)

public:
	explicit GUI_LanguageChooser(const QString& identifier);
	virtual ~GUI_LanguageChooser();

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

#endif // GUI_LANGUAGECHOOSER_H
