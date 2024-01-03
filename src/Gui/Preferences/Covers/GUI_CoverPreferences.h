/* GUI_CoverPreferences.h */

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

#ifndef GUI_COVERPREFERENCES_H
#define GUI_COVERPREFERENCES_H

#include "Gui/Preferences/PreferenceWidget.h"

UI_FWD(GUI_CoverPreferences)

class GUI_CoverPreferences :
	public Preferences::Base
{
	Q_OBJECT
	UI_CLASS(GUI_CoverPreferences)

	public:
		explicit GUI_CoverPreferences(const QString& identifier);
		~GUI_CoverPreferences() override;

		bool commit() override;
		void revert() override;

		QString actionName() const override;

	protected:
		void initUi() override;
		void retranslate() override;
		void skinChanged() override;

	private slots:
		void upClicked();
		void downClicked();
		void addClicked();
		void removeClicked();

		void currentRowChanged(int row);
		void deleteCoversFromDb();
		void deleteCoverFiles();
		void fetchCoversFromWWWTriggered(bool b);

		void saveCoverToLibraryToggled(bool b);
		void coverTemplateEdited(const QString& text);
};

#endif // GUI_CoverPreferences_H
