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
	enum class ViewType : quint8;
	/**
	 * @brief The GUI_LocalLibrary class
	 * @ingroup GuiLibrary
	 */
	class GUI_LocalLibrary :
			public GUI_AbstractLibrary
	{
		Q_OBJECT
		UI_CLASS(GUI_LocalLibrary)
		PIMPL(GUI_LocalLibrary)

	public:
		explicit GUI_LocalLibrary(LibraryId id, QWidget* parent=nullptr);
		virtual ~GUI_LocalLibrary() override;

		QMenu*		menu() const;
		QFrame*		headerFrame() const;

	protected:
		bool hasSelections() const override;
		void showEvent(QShowEvent* e) override;

		TableView* lvArtist() const override;
		TableView* lvAlbum() const override;
		TableView* lvTracks() const override;

		SearchBar* leSearch() const override;
		QList<Filter::Mode> searchOptions() const override;

		void queryLibrary() override;

		void languageChanged() override;
		void skinChanged() override;

	private:
		void checkViewState();
		void checkReloadStatus();
		void checkFileExtensionBar();

	private slots:
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
		void nameChanged(LibraryId id);
		void pathChanged(LibraryId id);

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

