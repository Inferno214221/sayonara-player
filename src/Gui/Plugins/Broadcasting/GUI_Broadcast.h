/* GUI_Broadcast.h */

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

#ifndef GUI_BROADCAST_H
#define GUI_BROADCAST_H

#include "Gui/Plugins/PlayerPluginBase.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_Broadcast)

class GUI_Broadcast :
		public PlayerPlugin::Base
{
	Q_OBJECT
	PIMPL(GUI_Broadcast)
	UI_CLASS(GUI_Broadcast)

public:
	explicit GUI_Broadcast(QWidget *parent=nullptr);
	virtual ~GUI_Broadcast();

	QString get_name() const override;
	QString get_display_name() const override;

private slots:
	void connection_established(const QString& ip);
	void connection_closed(const QString& ip);
	void can_listen_changed(bool b);

	void dismiss_clicked();
	void dismiss_all_clicked();
	void combo_changed(int idx);
	void retry();
	void mp3_enc_found();

private:
	void dismiss_at(int idx);
	void set_status_label();

	void retranslate_ui() override;
	void init_ui() override;

	bool check_dismiss_visible() const;
	bool check_dismiss_all_visible() const;
	void update_dismiss_buttons();

	void start_server();
};

#endif // GUI_BROADCAST_H
