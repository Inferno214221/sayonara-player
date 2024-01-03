/* GUI_History.h
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef GUI_HISTORY_H
#define GUI_HISTORY_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Session/SessionUtils.h"
#include "Utils/Pimpl.h"

class QFrame;
class QDate;

UI_FWD(GUI_History)

namespace Session
{
	class Manager;
}

class LibraryPlaylistInteractor;
class GUI_History :
	public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_History)
	UI_CLASS_SHARED_PTR(GUI_History)

	public:
		GUI_History(LibraryPlaylistInteractor* libraryPlaylistInteractor,
		            Session::Manager* sessionManager,
		            QWidget* parent = nullptr);
		~GUI_History() override;

		[[nodiscard]] QFrame* header() const;

	private:
		void initShortcuts();
		void requestData(int index);
		void assureOneTable();
		void loadSelectedDateRange();

	private slots: // NOLINT(readability-redundant-access-specifiers)
		void scrollToTop();
		void scrollToBottom();
		void loadMore();
		void dateRangeClicked();
		void clearRangeClicked();
		void calendarFinished();
		void clearAllHistoryClicked();
		void clearOldHistoryClicked();

	protected:
		void languageChanged() override;
};

#endif // GUI_HISTORY_H
