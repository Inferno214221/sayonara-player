/* GUI_PlayerMenubar.h */

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

#ifndef GUI_PLAYERMENUBAR_H
#define GUI_PLAYERMENUBAR_H

#include "Gui/Utils/Shortcuts/ShortcutIdentifier.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Pimpl.h"

#include <QMenuBar>

namespace Library
{
	class Container;
}

class Menubar :
	public Gui::WidgetTemplate<QMenuBar>
{
	Q_OBJECT
	PIMPL(Menubar)

signals:
	void sig_close_clicked();
	void sig_minimize_clicked();
	void sig_logger_clicked();

public:
	explicit Menubar(QWidget* parent=nullptr);
	~Menubar() override;

	void insert_player_plugin_action(QAction* action);
	void insert_preference_action(QAction* action);

	void show_library_action(bool visible);
	void set_show_library_action_enabled(bool b);
	void show_library_menu(bool b);

private:
	void init_connections();
	void style_changed();

protected:
	void language_changed() override;
	void skin_changed() override;

private slots:
	void open_dir_clicked();
	void open_files_clicked();
	void shutdown_clicked();
	void close_clicked();
	void minimize_clicked();
	void skin_toggled(bool b);
	void big_cover_toggled(bool b);
	void show_library_toggled(bool b);
	void show_fullscreen_toggled(bool b);
	void help_clicked();
	void about_clicked();
	void shortcut_changed(ShortcutIdentifier identifier);
	QAction* current_library_changed(Library::Container* library);
};


#endif // GUI_PLAYERMENUBAR_H
