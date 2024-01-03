/* TrackView.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef LIBRARYTRACKVIEW_H
#define LIBRARYTRACKVIEW_H

#include "TableView.h"
#include "Utils/Pimpl.h"
#include "Utils/Library/Sortorder.h"

class AbstractLibrary;
namespace Library
{
	/**
	 * @brief The TrackView class
	 * @ingroup GuiLibrary
	 */
	class TrackView :
		public TableView
	{
		Q_OBJECT
		PIMPL(TrackView)

		public:
			explicit TrackView(QWidget* parent = nullptr);
			~TrackView() override;

		private:
			AbstractLibrary* library() const override;
			//from Library::TableView
			void initView(AbstractLibrary* library) override;

			ColumnHeaderList columnHeaders() const override;
			QByteArray columnHeaderState() const override;
			void saveColumnHeaderState(const QByteArray& state) override;

			bool autoResizeState() const override;
			void saveAutoResizeState(bool b) override;

			SortOrder sortorder() const override;
			void applySortorder(SortOrder s) override;

			ContextMenu::Entries contextMenuEntries() const override;

			// from Library::ItemView
			void playClicked() override;
			void playNewTabClicked() override;
			void playNextClicked() override;
			void appendClicked() override;
			void selectedItemsChanged(const IndexSet& lst) override;
			void refreshClicked() override;

			bool isMergeable() const override;
			MD::Interpretation metadataInterpretation() const override;
	};
}

#endif // LIBRARYTRACKVIEW_H
