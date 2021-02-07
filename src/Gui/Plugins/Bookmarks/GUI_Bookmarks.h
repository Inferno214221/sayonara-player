/* GUI_Bookmarks.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "Gui/Plugins/PlayerPluginBase.h"
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
	explicit GUI_Bookmarks(Bookmarks* bookmarks, QWidget* parent=nullptr);
	~GUI_Bookmarks() override;

	QString name() const override;
	QString displayName() const override;

private:
	void retranslate() override;
	void initUi() override;


private slots:
	void currentIndexChanged(int currentIndex);
	void nextClicked();
	void previousClicked();
	void newClicked();
	void deleteClicked();
	void loopToggled(bool);

	void previousChanged(const Bookmark& bookmark);
	void nextChanged(const Bookmark& bookmark);

	void disablePrevious();
	void disableNext();

	void bookmarksChanged();
};

#endif // GUI_BOOKMARKS_H
