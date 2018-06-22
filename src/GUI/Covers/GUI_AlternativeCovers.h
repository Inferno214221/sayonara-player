/* GUI_AlternativeCovers.h */

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


/*
 * GUI_AlternativeCovers.h
 *
 *  Created on: Jul 1, 2011
 *      Author: Lucio Carreras
 */

#ifndef GUI_ALTERNATE_COVERS_H_
#define GUI_ALTERNATE_COVERS_H_

#include "GUI/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

namespace Cover
{
	class Location;
}


/**
 * @brief The GUI_AlternativeCovers class
 * @ingroup GUICovers
 */

UI_FWD(GUI_AlternativeCovers)

class GUI_AlternativeCovers :
		public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_AlternativeCovers)
	UI_CLASS(GUI_AlternativeCovers)

public:
	explicit GUI_AlternativeCovers(QWidget* parent=nullptr);
	virtual ~GUI_AlternativeCovers();

public slots:
	void start(const Cover::Location& cl);

signals:
	void sig_cover_changed(const Cover::Location& cl);

private slots:
	void ok_clicked();
	void apply_clicked();
	void search_clicked();
	void cover_pressed(const QModelIndex& idx);
	void open_file_dialog();
	void cl_new_cover(const QString& cover_path);
	void cl_finished(bool);

private:
	void reset();
	void connect_and_start();
	void init_combobox();

protected:
	void resizeEvent(QResizeEvent* e) override;
	void closeEvent(QCloseEvent* e) override;
	void language_changed() override;
};

#endif /* GUI_ALTERNATE_COVERS_H_ */
