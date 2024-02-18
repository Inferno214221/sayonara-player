/* ArtistView.h */

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

#ifndef ARTISTVIEW_H
#define ARTISTVIEW_H

#include "TableView.h"
#include "Utils/Pimpl.h"
#include "Utils/Library/Sortorder.h"

namespace Library
{
	class MergeData;

	class ArtistView :
		public TableView
	{
		Q_OBJECT
		PIMPL(ArtistView)

		public:
			explicit ArtistView(QWidget* parent = nullptr);
			~ArtistView() override;

		protected:
			[[nodiscard]] AbstractLibrary* library() const override;
			void selectedItemsChanged(const IndexSet& indexes) override;
			void playNextClicked() override;
			void appendClicked() override;
			void refreshClicked() override;
			void playClicked() override;
			void playNewTabClicked() override;
			void runMergeOperation(const Library::MergeData& mergedata) override;

			void initView(AbstractLibrary* library) override;
			void initContextMenu() override;

			[[nodiscard]] ColumnHeaderList columnHeaders() const override;
			[[nodiscard]] QByteArray columnHeaderState() const override;
			void saveColumnHeaderState(const QByteArray& state) override;

			[[nodiscard]] VariableSortorder sortorder() const override;
			void applySortorder(VariableSortorder s) override;

			[[nodiscard]] bool autoResizeState() const override;
			void saveAutoResizeState(bool b) override;

			// ItemView
			[[nodiscard]] bool isMergeable() const override;
			[[nodiscard]] MD::Interpretation metadataInterpretation() const override;

			void languageChanged() override;
			[[nodiscard]] SearchModel* searchModel() const override;

		private slots:
			void useClearButtonChanged();
			void showAlbumArtistsChanged();
			void albumArtistsTriggered(bool b);
	};
}

#endif // ARTISTVIEW_H
