/* GUI_LastFmPreferences.h */

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


/*
 * GUI_LastFmPreferences.h
 *
 *  Created on: Apr 21, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef GUI_LASTFM_PREFERENCES_H
#define GUI_LASTFM_PREFERENCES_H

#include "Gui/Preferences/PreferenceWidget.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_LastFmPreferences)

namespace LastFM
{
	class Base;
}

class GUI_LastFmPreferences :
	public Preferences::Base
{
	Q_OBJECT
	UI_CLASS(GUI_LastFmPreferences)
	PIMPL(GUI_LastFmPreferences)

	public:
		explicit GUI_LastFmPreferences(const QString& identifier, LastFM::Base* lastFm);
		~GUI_LastFmPreferences() override;

		bool commit() override;
		void revert() override;

		[[nodiscard]] QString actionName() const override;

	protected:
		void initUi() override;
		void retranslate() override;

	private slots:
		void loginClicked();
		void activeChanged(bool active);
		void loginFinished(bool success);
};

#endif /* GUI_LastFmPreferences_H_ */
