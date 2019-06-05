/* GUI_LastFM.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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
 * GUI_LastFM.h
 *
 *  Created on: Apr 21, 2011
 *      Author: Lucio Carreras
 */

#ifndef GUI_LASTFM_H_
#define GUI_LASTFM_H_

#include "Gui/Preferences/PreferenceWidget.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_LastFM)

namespace LastFM
{
	class Base;
}

class GUI_LastFM :
		public Preferences::Base
{
	Q_OBJECT
	UI_CLASS(GUI_LastFM)
	PIMPL(GUI_LastFM)

public:
	explicit GUI_LastFM(const QString& identifier, LastFM::Base* lfm);
	virtual ~GUI_LastFM();

	bool commit() override;
	void revert() override;

	QString action_name() const override;

protected:
	void init_ui() override;
	void retranslate_ui() override;

private slots:
	void btn_login_clicked();
	void active_changed(bool active);
	void logged_in(bool success);
};

#endif /* GUI_LASTFM_H_ */
