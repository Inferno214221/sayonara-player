/* GUI_IconPreferences.h */

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



#ifndef GUI_ICONPREFERENCES_H
#define GUI_ICONPREFERENCES_H

#include "Gui/Utils/Widgets/Widget.h"
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
		explicit GUI_IconPreferences(QWidget* parent = nullptr);
		virtual ~GUI_IconPreferences();

	protected:
		void languageChanged() override;
		void showEvent(QShowEvent* e) override;

	public:
		QString actionName() const;

		bool commit();
		void revert();

	private:
		void initUi();

	private slots:
		void themeChanged(const QString& theme);
		void radioButtonToggled(bool b);
};

#endif // GUI_ICONPREFERENCES_H
