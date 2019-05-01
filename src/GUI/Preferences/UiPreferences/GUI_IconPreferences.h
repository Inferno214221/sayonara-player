/* GUI_IconPreferences.h */

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



#ifndef GUI_ICONPREFERENCES_H
#define GUI_ICONPREFERENCES_H

#include "GUI/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_IconPreferences)

class QWidget;
class GUI_IconPreferences :
		public Gui::Widget
{
	Q_OBJECT
	PIMPL(GUI_IconPreferences)
	UI_CLASS(GUI_IconPreferences)


public:
	explicit GUI_IconPreferences(QWidget* parent=nullptr);
	virtual ~GUI_IconPreferences();

protected:
	void language_changed() override;
	void showEvent(QShowEvent* e) override;

public:
	QString action_name() const;

	bool commit();
	void revert();

private:
	void init_ui();

private slots:
	void theme_changed(const QString& theme);
};

#endif // GUI_ICONPREFERENCES_H
