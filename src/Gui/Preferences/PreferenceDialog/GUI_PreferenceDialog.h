/* GUI_PreferenceDialog.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef GUI_PreferenceDialog_H
#define GUI_PreferenceDialog_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/Widgets/Dialog.h"
#include "Components/Preferences/PreferenceRegistry.h"

namespace Preferences
{
	class Base;
	class Action;
}

UI_FWD(GUI_PreferenceDialog)

/**
 * @brief The Preference Dialog. Register new Preference dialogs with the register_preference_dialog() method.
 * @ingroup Preferences
 */
class GUI_PreferenceDialog :
		public Gui::Dialog,
		public PreferenceUi
{
	Q_OBJECT
	UI_CLASS(GUI_PreferenceDialog)
	PIMPL(GUI_PreferenceDialog)

	signals:
		void sig_error(const QString& error_message);

	public:
		explicit GUI_PreferenceDialog(QWidget* parent=nullptr);
		~GUI_PreferenceDialog();

		QString action_name() const;
		QAction* action();

		QList<QAction*> actions(QWidget* parent);

		void register_preference_dialog(Preferences::Base* dialog);
		void show_preference(const QString& identifier) override;

	protected slots:
		void commit_and_close();
		void row_changed(int row);

	protected:
		void init_ui();
		void language_changed() override;
		void showEvent(QShowEvent *e) override;

		void hide_all();

	private:
		bool commit();
		void revert();

};

#endif // GUI_PreferenceDialog_H
