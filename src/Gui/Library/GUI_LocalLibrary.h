/* GUI_LocalLibrary.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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
 *      Author: Lucio Carreras
 */

#ifndef GUI_LOCAL_LIBRARY_H_
#define GUI_LOCAL_LIBRARY_H_

#include "GUI_AbstractLibrary.h"
#include "Utils/Pimpl.h"
#include "Utils/Library/LibraryNamespaces.h"

UI_FWD(GUI_LocalLibrary)

namespace Library
{
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
		QFrame*		header_frame() const;

	protected:

		bool has_selections() const override;
		void showEvent(QShowEvent* e) override;

		TableView* lv_artist() const override;
		TableView* lv_album() const override;
		TableView* lv_tracks() const override;

		SearchBar* le_search() const override;
		QList<Filter::Mode> search_options() const override;

		void language_changed() override;
		void skin_changed() override;

	private:
		void check_view_state();
		void check_reload_status();
		void check_file_extension_bar();

	private slots:
		void tracks_loaded();
		void switch_album_view();
		void filter_changed();

		void progress_changed(const QString& type, int progress);

		void genre_selection_changed(const QStringList& genres);
		void invalid_genre_selected();

		void reload_library_deep_requested();
		void reload_library_requested();
		void reload_library_requested_with_quality(ReloadQuality quality);
		void reload_library_accepted(ReloadQuality quality);
		void reload_finished();

		void import_dirs_requested();
		void import_files_requested();
		void name_changed(LibraryId id);
		void path_changed(LibraryId id);

		// importer requests dialog
		void import_dialog_requested(const QString& target_dir);

		void splitter_artist_moved(int pos, int idx);
		void splitter_tracks_moved(int pos, int idx);
		void splitter_genre_moved(int pos, int idx);

		// reimplemented from Abstract Library
		TrackDeletionMode show_delete_dialog(int track_count) override;
		void clear_selections() override;

		void show_info_box();
	};
}

#endif /* GUI_LocalLibrary_H_ */

