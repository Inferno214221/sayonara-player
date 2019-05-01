/* GUI_Bookmarks.h */

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

#ifndef GUI_BOOKMARKS_H
#define GUI_BOOKMARKS_H

#include "Interfaces/PlayerPlugin/PlayerPluginBase.h"
#include "Utils/Pimpl.h"

class Bookmarks;
class Bookmark;

UI_FWD(GUI_Bookmarks)

/**
 * @brief The GUI_Bookmarks class
 * @ingroup Bookmarks
 */
class GUI_Bookmarks :
		public PlayerPlugin::Base
{
	Q_OBJECT
	UI_CLASS(GUI_Bookmarks)
	PIMPL(GUI_Bookmarks)

public:
	explicit GUI_Bookmarks(QWidget *parent=nullptr);
	virtual ~GUI_Bookmarks();

	QString get_name() const override;
	QString get_display_name() const override;


private:
	void retranslate_ui() override;
	void init_ui() override;


private slots:
	void combo_changed(int new_idx);
	void next_clicked();
	void prev_clicked();
	void new_clicked();
	void del_clicked();
	void loop_clicked(bool);

	void prev_changed(const Bookmark& bookmark);
	void next_changed(const Bookmark& bookmark);

	void disable_prev();
	void disable_next();

	void bookmarks_changed();
};

#endif // GUI_BOOKMARKS_H
