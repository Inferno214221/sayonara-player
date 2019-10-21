/* LocalLibrary.h */

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

#ifndef LocalLibrary_H
#define LocalLibrary_H

#include "AbstractLibrary.h"
#include "Utils/Pimpl.h"

class ReloadThread;

namespace Library
{
	class Importer;
	class Manager;
}

class LocalLibrary :
		public AbstractLibrary
{
	friend class Library::Manager;

	Q_OBJECT
	PIMPL(LocalLibrary)

signals:
	void sig_import_dialog_requested(const QString& target_dir);
	void sig_renamed(const QString& new_name);

protected:
	LocalLibrary(LibraryId id, QObject* parent=nullptr);

public:
	~LocalLibrary() override;

	bool set_library_path(const QString& path);
	bool set_library_name(const QString& name);

	QString			path() const;
	LibraryId		id() const;
	QString			name() const;
	Library::Importer* importer();

	bool is_reloading() const override;

public slots:
	void delete_tracks(const MetaDataList& v_md, Library::TrackDeletionMode answer) override;
	void reload_library(bool clear_first, Library::ReloadQuality quality) override;

	void refresh_artist() override;
	void refresh_albums() override;
	void refresh_tracks() override;

	void import_files(const QStringList& files) override;
	void import_files_to(const QStringList& files, const QString& target_dir);

protected slots:
	void reload_thread_new_block();
	void reload_thread_finished();
	void search_mode_changed();
	void show_album_artists_changed();
	void renamed(LibraryId id);

private:
	void get_all_artists(ArtistList& artists) const override;
	void get_all_artists_by_searchstring(Library::Filter filter, ArtistList& artists) const override;

	void get_all_albums(AlbumList& albums) const override;
	void get_all_albums_by_artist(IdList artist_ids, AlbumList& albums, Library::Filter filter) const override;
	void get_all_albums_by_searchstring(Library::Filter filter, AlbumList& albums) const override;

	int get_num_tracks() const override;
	void get_all_tracks(MetaDataList& v_md) const override;
	void get_all_tracks(const QStringList& paths, MetaDataList& v_md) const override;
	void get_all_tracks_by_artist(IdList artist_ids, MetaDataList& v_md, Library::Filter filter) const override;
	void get_all_tracks_by_album(IdList album_ids, MetaDataList& v_md, Library::Filter filter) const override;
	void get_all_tracks_by_searchstring(Library::Filter filter, MetaDataList& v_md) const override;
	void get_all_tracks_by_path(const QStringList& paths, MetaDataList& v_md) const override;

	void get_track_by_id(TrackID track_id, MetaData& md) const override;
	void get_album_by_id(AlbumId album_id, Album& album) const override;
	void get_artist_by_id(ArtistId artist_id, Artist& artist) const override;

	void apply_db_fixes();
	void init_reload_thread();
};

#endif // LocalLibrary_H
