/* GUI_LocalLibrary.h */

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


/*
 * GUI_LocalLibrary.h
 *
 *  Created on: Apr 24, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef GUI_LOCAL_LIBRARY_H_
#define GUI_LOCAL_LIBRARY_H_

#include "GUI_AbstractLibrary.h"
#include "Utils/Pimpl.h"
#include "Utils/Library/LibraryNamespaces.h"

UI_FWD(GUI_LocalLibrary)

namespace Library
{
	class Manager;

	enum class ViewType :
		quint8;

	class GUI_LocalLibrary :
		public GUI_AbstractLibrary
	{
		Q_OBJECT
		UI_CLASS(GUI_LocalLibrary)
		PIMPL(GUI_LocalLibrary)

		public:
			explicit GUI_LocalLibrary(LibraryId id, Library::Manager* libraryManager, QWidget* parent = nullptr);
			~GUI_LocalLibrary() override;

			[[nodiscard]] QMenu* menu() const;
			[[nodiscard]] QFrame* headerFrame() const;

		protected:
			[[nodiscard]] bool hasSelections() const override;
			void showEvent(QShowEvent* e) override;

			[[nodiscard]] TableView* lvArtist() const override;
			[[nodiscard]] TableView* lvAlbum() const override;
			[[nodiscard]] TableView* lvTracks() const override;
			[[nodiscard]] QList<QAbstractItemView*> allViews() const override;

			[[nodiscard]] SearchBar* leSearch() const override;
			[[nodiscard]] QList<Filter::Mode> searchOptions() const override;

			void queryLibrary() override;

			void languageChanged() override;
			void skinChanged() override;

		private:
			void initCoverView();
			void checkViewState();
			void checkMainSplitterStatus();
			void checkFileExtensionBar();

		private slots: // NOLINT(readability-redundant-access-specifiers)
			void tracksLoaded();
			void switchViewType();
			void selectNextViewType();

			void progressChanged(const QString& type, int progress);

			void genreSelectionChanged(const QStringList& genres);
			void invalidGenreSelected();

			void reloadLibraryDeepRequested();
			void reloadLibraryRequested();
			void reloadLibraryRequestedWithQuality(ReloadQuality quality);
			void reloadLibraryAccepted(ReloadQuality quality);
			void reloadLibrary(ReloadQuality quality);
			void reloadFinished();

			void importDirsRequested();
			void importFilesRequested();
			void nameChanged(const QString& newName);
			void pathChanged(const QString& newPath);

			// importer requests dialog
			void importDialogRequested(const QString& targetDirectory);

			void splitterArtistMoved(int pos, int idx);
			void splitterTracksMoved(int pos, int idx);
			void splitterGenreMoved(int pos, int idx);

			// reimplemented from Abstract Library
			TrackDeletionMode showDeleteDialog(int track_count) override;
			void clearSelections() override;

			void showInfoBox();
	};
}

#endif /* GUI_LocalLibrary_H_ */

