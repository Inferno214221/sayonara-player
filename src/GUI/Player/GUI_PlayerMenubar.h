/* GUI_PlayerMenubar.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include <QMenuBar>
#include "GUI/Utils/Widgets/WidgetTemplate.h"
#include "GUI/Utils/Shortcuts/ShortcutWidget.h"
#include "Utils/Pimpl.h"

class QMenu;
class QAction;

class Menubar :
	public Gui::WidgetTemplate<QMenuBar>,
	public ShortcutWidget
{
	Q_OBJECT
	PIMPL(Menubar)

signals:
	void sig_close_clicked();
	void sig_minimize_clicked();
	void sig_logger_clicked();

public:
	explicit Menubar(QWidget* parent=nullptr);
	~Menubar();

	void insert_player_plugin_action(QAction* action);
	void insert_preference_action(QAction* action);
	QAction* update_library_action(QMenu* new_library_menu, const QString& name);
	void show_library_action(bool visible);
	void set_show_library_action_enabled(bool b);

private:
	void init_connections();

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
	void show_library_toggled(bool b);
	void show_fullscreen_toggled(bool b);
	void help_clicked();
	void about_clicked();
	void awa_translators_finished();
	void shortcut_changed(ShortcutIdentifier identifier);
};


#endif // GUI_PLAYERMENUBAR_H
