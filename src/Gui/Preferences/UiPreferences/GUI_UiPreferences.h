/* GUI_UiPreferences.h */

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

#ifndef GUI_UI_PREFERENCES_H_
#define GUI_UI_PREFERENCES_H_

#include "Gui/Preferences/PreferenceWidget.h"

UI_FWD(GUI_UiPreferences)

class GUI_UiPreferences :
	public Preferences::Base
{
	Q_OBJECT
	PIMPL(GUI_UiPreferences)
	UI_CLASS_SHARED_PTR(GUI_UiPreferences)

	public:
		explicit GUI_UiPreferences(const QString& identifier);
		~GUI_UiPreferences() override;

		[[nodiscard]] QString actionName() const override;

	protected:
		void retranslate() override;
		bool commit() override;
		void revert() override;
		void initUi() override;

	private:
		void styleChanged();

	private slots:
		void editCssClicked();

};

#endif
