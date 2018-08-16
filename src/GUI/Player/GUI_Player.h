/* GUI_Simpleplayer.h */

/* Copyright (C) 2011-2017 Lucio Carreras
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

#ifndef GUI_SIMPLEPLAYER_H
#define GUI_SIMPLEPLAYER_H

#include "Components/PlayManager/PlayState.h"
#include "Utils/Message/MessageReceiverInterface.h"
#include "Utils/Pimpl.h"
#include "GUI/Utils/GuiClass.h"
#include "GUI/Utils/Widgets/Widget.h"

#include <QSystemTrayIcon>

class GUI_TrayIcon;
class GUI_Logger;
class MetaData;
class PreferenceDialog;

class QAction;
class QMessageBox;
class QTranslator;

UI_FWD(GUI_Player)

namespace PlayerPlugin
{
	class Handler;
	class Base;
}

class GUI_Player :
		public Gui::MainWindow,
		public MessageReceiverInterface
{
	Q_OBJECT
	PIMPL(GUI_Player)
	UI_CLASS(GUI_Player)

signals:
	void sig_player_closed();

public:
	explicit GUI_Player(QWidget *parent=nullptr);
	~GUI_Player();

	void register_preference_dialog(QAction* dialog_action);
	void request_shutdown();

private:
	bool init_translator(const QString& file, const QString& dir);
	void init_tray_actions();
	void init_connections();
	void init_sizes();
	void init_main_splitter();

	void closeEvent(QCloseEvent* e) override;
	void resizeEvent(QResizeEvent* e) override;
	void moveEvent(QMoveEvent* e) override;

	void language_changed() override;
	void skin_changed() override;
	void show_library_changed();
	void show_library(bool is_library_visible, bool was_library_visible=false);
	void fullscreen_changed();
	void init_controlstyle();
	void controlstyle_changed();

	void set_total_time_label(MilliSeconds length_ms);
	void set_cur_pos_label(int val);
	void set_cover_location(const MetaData& md);
	void set_standard_cover();

	// Methods for other mudules to display info/warning/error
	Message::Answer error_received(const QString &error, const QString &sender_name=QString()) override;
	Message::Answer warning_received(const QString &error, const QString &sender_name=QString()) override;
	Message::Answer info_received(const QString &error, const QString &sender_name=QString()) override;
	Message::Answer question_received(const QString &info, const QString &sender_name=QString(), Message::QuestionType type=Message::QuestionType::YesNo) override;


private slots:
	void playstate_changed(PlayState state);
	void play_error(const QString& message);

	void splitter_main_moved(int pos, int idx);
	void splitter_controls_moved(int pos, int idx);

	void current_library_changed(const QString& name);
	void check_library_menu_action();
	void splitter_painted();

	void awa_version_finished();

	void minimize();
	void minimize_to_tray();
	void really_close();

	void tray_icon_activated(QSystemTrayIcon::ActivationReason reason);

	/* Plugins */
	void plugin_added(PlayerPlugin::Base* plugin);
	void plugin_opened();
	void plugin_closed();
	void plugin_action_triggered(bool b);
};

#endif // GUI_SIMPLEPLAYER_H
