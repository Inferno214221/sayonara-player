/* HistoryEntryWidget.h
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

#ifndef HISTORYENTRYWIDGET_H
#define HISTORYENTRYWIDGET_H

#include "Utils/Pimpl.h"
#include "Utils/Session/SessionUtils.h"

#include "Gui/Utils/Widgets/Widget.h"

namespace Session
{
	class Manager;
}

class LibraryPlaylistInteractor;
class HistoryEntryWidget :
	public Gui::Widget
{
	Q_OBJECT
	PIMPL(HistoryEntryWidget)

	public:
		HistoryEntryWidget(LibraryPlaylistInteractor* libraryPlaylistInteractor,
		                   Session::Manager* sessionManager,
		                   Session::Timecode timecode,
		                   QWidget* parent = nullptr);
		~HistoryEntryWidget() override;
		[[nodiscard]] Session::Id id() const;

	protected:
		void languageChanged() override;

	private slots:
		void rowcountChanged();
};

#endif // HISTORYENTRYWIDGET_H
