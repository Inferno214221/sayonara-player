/* LocalLibraryMenu.h */

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

#ifndef LOCALLIBRARYMENU_H
#define LOCALLIBRARYMENU_H

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/Shortcuts/ShortcutIdentifier.h"
#include "Utils/Pimpl.h"

#include <QMenu>
#include <QAction>

namespace Gui
{
	class PreferenceAction;
}

namespace Library
{
	/**
	 * @brief A menu in the player's menubar containing
	 * some library actions
	 * @ingroup GuiLibrary
	 */
	class LocalLibraryMenu :
			public Gui::WidgetTemplate<QMenu>
	{
		Q_OBJECT
		PIMPL(LocalLibraryMenu)

	signals:
		void sig_reload_library();
		void sig_import_file();
		void sig_import_folder();
		void sig_info();

		void sig_name_changed(const QString& name);
		void sig_path_changed(const QString& path);

	public:
		explicit LocalLibraryMenu(const QString& name, const QString& path, QWidget* parent=nullptr);
		~LocalLibraryMenu() override;

		void refresh_name(const QString& name);
		void refresh_path(const QString& path);

		void set_show_album_covers_checked(bool checked);
		void set_library_busy(bool b);
		void set_library_empty(bool b);

		void add_preference_action(Gui::PreferenceAction* action);

	private:
		void init_menu();

	protected:
		void language_changed() override;
		void skin_changed() override;
		void shortcut_changed(ShortcutIdentifier identifier);

	private slots:
		void show_album_covers_changed();
		void show_album_covers_triggered(bool b);

		void show_album_artists_changed();
		void show_album_artists_triggered(bool b);

		void realtime_search_changed();
		void realtime_search_triggered(bool b);

		void edit_clicked();
		void edit_accepted();
	};
}

#endif // LOCALLIBRARYMENU_H
