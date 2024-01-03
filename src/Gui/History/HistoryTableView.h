/* HistoryTableView.h
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

#ifndef HISTORYTABLEVIEW_H
#define HISTORYTABLEVIEW_H

#include "Gui/Utils/Widgets/Dragable.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Pimpl.h"
#include "Utils/Session/SessionUtils.h"

#include <QTableView>

namespace Session
{
	class Manager;
}

class LibraryPlaylistInteractor;

class HistoryTableView :
	public Gui::WidgetTemplate<QTableView>,
	public Gui::Dragable
{
	Q_OBJECT
	PIMPL(HistoryTableView)

	signals:
		void sigRowcountChanged();

	public:
		HistoryTableView(LibraryPlaylistInteractor* libraryPlaylistInteractor,
		                 Session::Manager* sessionManager,
		                 Session::Timecode timecode,
		                 QWidget* parent = nullptr);
		~HistoryTableView() override;

		[[nodiscard]] int rows() const;

	protected:
		void skinChanged() override;
		void resizeEvent(QResizeEvent* e) override;
		void showEvent(QShowEvent* e) override;
		void contextMenuEvent(QContextMenuEvent* e) override;

	private slots:
		void rowcountChanged();
		void appendTriggered();
		void playNewTabTriggered();
		void playNextTriggered();
		void playTriggered();

	private: // NOLINT(readability-redundant-access-specifiers)
		void initContextMenu();
};

#endif // HISTORYTABLEVIEW_H
