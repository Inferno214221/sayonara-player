/* GUI_StationSearcher.h */

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



#ifndef GUISTATIONSEARCHER_H
#define GUISTATIONSEARCHER_H

#include "GUI/Utils/Widgets/Dialog.h"
#include "Components/Streaming/StationSearcher/StationSearcher.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_StationSearcher)

class GUI_StationSearcher :
		public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_StationSearcher)
	UI_CLASS(GUI_StationSearcher)

signals:
	void sig_stream_selected(const QString& name, const QString& url);

public:
	GUI_StationSearcher(QWidget* parent=nullptr);
	~GUI_StationSearcher();

private:
	void init_line_edit();
	void check_listen_button();
	void clear_stations();
	void clear_streams();
	void change_mode(StationSearcher::Mode mode);


private slots:
	void search_clicked();
	void search_prev_clicked();
	void search_next_clicked();
	void listen_clicked();

	void search_text_changed(const QString& text);
	void stations_fetched();

	void station_changed();
	void stream_changed();

protected:
	void showEvent(QShowEvent* e) override;
	void closeEvent(QCloseEvent* e) override;

	void language_changed() override;
	void skin_changed() override;
};


#endif // STATIONSEARCHER_H
