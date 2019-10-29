/* TagEdit.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "Editor.h"
#include "Expression.h"
#include "ChangeNotifier.h"
#include "ChangeInformation.h"

#include "Components/MetaDataInfo/MetaDataInfo.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverChangeNotifier.h"
#include "Components/Directories/MetaDataScanner.h"
#include "Database/CoverConnector.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Set.h"
#include "Utils/FileUtils.h"

#include "Utils/Tagging/Tagging.h"
#include "Utils/Tagging/TaggingCover.h"

#include "Utils/MetaData/Genre.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Library/Filter.h"
#include "Utils/Library/Sortorder.h"

#include "Database/LibraryDatabase.h"
#include "Database/Connector.h"

#include <QHash>
#include <QFileInfo>

namespace Algorithm=Util::Algorithm;

using namespace Tagging;

struct Editor::Private
{
	QList<ChangeInformation> change_info;

	// used for retrieving a list of all changed metadata
	MetaDataList			v_md_before_change;
	MetaDataList			v_md_after_change;

	QMap<QString, Editor::FailReason>	failed_files;
};

Editor::Editor(QObject *parent) :
	QObject(parent),
	DB::ConnectorConsumer()
{
	m = Pimpl::make<Editor::Private>();
}

Editor::Editor(const MetaDataList& v_md, QObject* parent) :
	Editor(parent)
{
	set_metadata(v_md);
}

Editor::~Editor() = default;

void Editor::update_track(int idx, const MetaData& md)
{
	if(Util::between(idx, m->change_info))
	{
		m->change_info[idx].update(md);
	}
}

void Editor::undo(int idx)
{
	if(Util::between(idx, m->change_info))
	{
		m->change_info[idx].undo();
	}
}

void Editor::undo_all()
{
	for(auto it=m->change_info.begin(); it != m->change_info.end(); it++)
	{
		it->undo();
	}
}

MetaData Editor::metadata(int idx) const
{
	if(Util::between(idx, m->change_info))
	{
		return m->change_info[idx].current_metadata();
	}

	return MetaData();
}

MetaDataList Editor::metadata() const
{
	MetaDataList v_md; v_md.reserve( size_t(m->change_info.size()) );
	for(auto it=m->change_info.begin(); it != m->change_info.end(); it++)
	{
		v_md.push_back(it->current_metadata());
	}

	return v_md;
}

bool Editor::apply_regex(const QString& regex, int idx)
{
	if(!Util::between(idx, m->change_info)) {
		return false;
	}

	MetaData& md = m->change_info[idx].current_metadata();
	Expression e(regex, md.filepath());
	if(!e.is_valid()) {
		return false;
	}

	bool b = e.apply(md);
	m->change_info[idx].set_changed(b);

	return true;
}

int Editor::count() const
{
	return m->change_info.count();
}

bool Editor::has_changes() const
{
	return Algorithm::contains(m->change_info, [](const ChangeInformation& info){
		return info.has_changes();
	});
}

void Editor::add_genre(int idx, const Genre& genre)
{
	if(!Util::between(idx, m->change_info))
	{
		MetaData& md = m->change_info[idx].current_metadata();
		if(md.add_genre(genre))
		{
			m->change_info[idx].set_changed(true);
		}
	}
}

void Editor::delete_genre(int idx, const Genre& genre)
{
	if(!Util::between(idx, m->change_info))
	{
		MetaData& md = m->change_info[idx].current_metadata();
		if(md.remove_genre(genre))
		{
			m->change_info[idx].set_changed(true);
		}
	}
}

void Editor::rename_genre(int idx, const Genre& genre, const Genre& new_genre)
{
	delete_genre(idx, genre);
	add_genre(idx, new_genre);
}


void Editor::set_metadata(const MetaDataList& v_md)
{
	m->change_info.clear();
	m->change_info.reserve(int(v_md.size()));

	for(const MetaData& md : v_md)
	{
		m->change_info << ChangeInformation(md);
	}

	m->failed_files.clear();

	emit sig_metadata_received(v_md);
}

bool Editor::is_cover_supported(int idx) const
{
	if(!Util::between(idx, m->change_info))
	{
		const MetaData& md = m->change_info[idx].original_metadata();
		return Tagging::Covers::is_cover_supported(md.filepath());
	}

	return false;
}

bool Editor::can_load_entire_album() const
{
	Util::Set<AlbumId> album_ids;

	for(const ChangeInformation& info : m->change_info)
	{
		album_ids << info.original_metadata().album_id;
		if(album_ids.size() > 1)
		{
			return false;
		}
	}

	return (album_ids.size() == 1);
}

void Editor::load_entire_album()
{
	Util::Set<AlbumId> album_ids;
	for(const ChangeInformation& info : m->change_info)
	{
		album_ids << info.original_metadata().album_id;
	}

	if(album_ids.size() != 1){
		return;
	}

	AlbumId id = album_ids.first();
	if(id < 0 && m->change_info.size() > 0)
	{
		emit sig_started();
		emit sig_progress(-1);

		QString dir, filename;
		QString path = m->change_info[0].original_metadata().filepath();
		Util::File::split_filename(path, dir, filename);

		using Directory::MetaDataScanner;
		auto* t = new QThread();
		auto* worker = new MetaDataScanner({dir}, true);
		worker->moveToThread(t);

		connect(t, &QThread::finished, t, &QObject::deleteLater);
		connect(t, &QThread::started, worker, &MetaDataScanner::start);
		connect(worker, &MetaDataScanner::sig_finished, t, &QThread::quit);
		connect(worker, &MetaDataScanner::sig_finished, this, &Editor::load_entire_album_finished);

		t->start();
	}

	else
	{
		MetaDataList v_md;

		auto* db = db_connector();
		auto* ldb = db->library_db(-1, 0);

		ldb->getAllTracksByAlbum(IdList{id}, v_md, ::Library::Filter(), -1);
		v_md.sort(::Library::SortOrder::TrackDiscnumberAsc);
		set_metadata(v_md);
	}
}

void Editor::load_entire_album_finished()
{
	auto* worker = static_cast<Directory::MetaDataScanner*>(sender());

	MetaDataList v_md = worker->metadata();
	if(!v_md.isEmpty())
	{
		this->set_metadata(v_md);
	}

	worker->deleteLater();

	emit sig_finished();
}

void Editor::apply_artists_and_albums_to_md()
{
	QHash<QString, ArtistId> artist_map;
	QHash<QString, AlbumId>	album_map;

	auto* db = db_connector();
	auto* ldb = db->library_db(-1, 0);

	{ // load_all_albums
		AlbumList albums;
		ldb->getAllAlbums(albums, true);
		for(auto it=albums.begin(); it != albums.end(); it++)
		{
			if(album_map.contains(it->name()))
			{
				Album album = *it;
				sp_log(Log::Warning, this) << "Album " << it->name() << " already exists";
				continue;
			}

			album_map[it->name()] = it->id;
		}
	}

	{ // load all artists
		ArtistList artists;
		ldb->getAllArtists(artists, true);
		for(auto it=artists.begin(); it != artists.end(); it++)
		{
			if(artist_map.contains(it->name()))
			{
				sp_log(Log::Warning, this) << "Artist " << it->name() << " already exists";
				continue;
			}

			artist_map[it->name()] = it->id;
		}
	}

	Util::Set<QString> insert_artists, insert_albums;
	{ // scan for unknown artists/albums
		for(auto it=m->change_info.begin(); it != m->change_info.end(); it++)
		{
			const MetaData& md = it->current_metadata();
			if(!artist_map.contains(md.artist())) {
				insert_artists << md.artist();
			}

			if(!artist_map.contains(md.album_artist())){
				insert_artists << md.album_artist();
			}

			if(!album_map.contains(md.album())){
				insert_albums << md.album();
			}
		}
	}

	db->transaction();

	{ // insert unknown artists
		if(!insert_artists.isEmpty())
		{
			for(const QString& artist : insert_artists)
			{
				ArtistId id = ldb->insertArtistIntoDatabase(artist);
				if(id >= 0)	{
					artist_map.insert(artist, id);
				} else {
					sp_log(Log::Warning, this) << "Invalid artist id";
				}
			}
		}
	}

	{ // insert unknown albums
		if(!insert_albums.isEmpty())
		{
			for(const QString& album : insert_albums)
			{
				AlbumId id = ldb->insertAlbumIntoDatabase(album);
				if(id >= 0){
					album_map.insert(album, id);
				} else {
					sp_log(Log::Warning, this) << "Invalid album id";
				}
			}
		}
	}

	db->commit();

	for(auto it=m->change_info.begin(); it != m->change_info.end(); it++)
	{
		MetaData md = it->current_metadata();

		md.album_id = album_map[md.album()];
		md.artist_id = artist_map[md.artist()];
		md.set_album_artist_id(artist_map[md.album_artist()]);

		it->update(md);
	}
}

void Editor::update_cover(int idx, const QPixmap& cover)
{
	if(cover.isNull() || !Util::between(idx, m->change_info)) {
		return;
	}

	bool cover_supported = is_cover_supported(idx);
	if(!cover_supported) {
		return;
	}

	m->change_info[idx].update_cover(cover);
}

bool Editor::has_cover_replacement(int idx) const
{
	return (Util::between(idx, m->change_info) && m->change_info[idx].has_new_cover());
}

void Editor::commit()
{
	emit sig_started();

	auto* db = db_connector();
	auto* db_covers = db->cover_connector();
	auto* ldb = db->library_db(-1, 0);

	m->v_md_before_change.clear();
	m->v_md_after_change.clear();

	const auto changed_tracks = std::count_if(m->change_info.begin(), m->change_info.end(), [](const ChangeInformation& info){
		return (info.has_changes());
	});

	const auto changed_covers = std::count_if(m->change_info.begin(), m->change_info.end(), [](const ChangeInformation& info){
		return (info.has_new_cover());
	});

	if((changed_tracks + changed_covers) == 0)
	{
		emit sig_finished();
		return;
	}

	sp_log(Log::Debug, this) << "Changing " << changed_tracks << " tracks and " << changed_covers << " covers";

	apply_artists_and_albums_to_md();

	db->transaction();

	int progress = 0;
	for(auto it=m->change_info.begin(); it != m->change_info.end(); it++)
	{
		emit sig_progress( ((progress + 1) * 100) / (changed_tracks + changed_covers));

		/* Normal tags changed */
		if(it->has_changes())
		{
			bool success;
			{ // write Metadata to file
				success = Tagging::Utils::setMetaDataOfFile(it->current_metadata());
				if(!success)
				{
					QString filepath = it->original_metadata().filepath();
					QFileInfo fi(filepath);
					if(!fi.exists())
					{
						m->failed_files.insert(filepath, FailReason::FileNotFound);
						sp_log(Log::Warning, this) << "Failed to write tags to file: File not found";
					}

					else if(!fi.isWritable())
					{
						m->failed_files.insert(filepath, FailReason::FileNotWriteable);
						sp_log(Log::Warning, this) << "Failed to write tags to file: File not writeable";
					}

					else {
						m->failed_files.insert(filepath, FailReason::TagLibError);
					}
				}
			}

			if(success)
			{ // write changed to db
				const MetaData& org_md = it->original_metadata();
				const MetaData& cur_md = it->current_metadata();
				if( !cur_md.is_extern && cur_md.id >= 0 )
				{
					if(ldb->updateTrack(cur_md))
					{
						m->v_md_before_change << org_md;
						m->v_md_after_change << cur_md;

						// update track
						it->apply();
					}
				}
			}

			progress++;
		}

		/* Covers changed */
		if(it->has_new_cover())
		{
			QPixmap pm = it->cover();
			if(pm.size().width() > 1000 || pm.size().height() > 1000)
			{
				pm = pm.scaled(QSize(1000, 1000), Qt::KeepAspectRatio, Qt::SmoothTransformation);
			}

			const MetaData& md = it->current_metadata();
			Cover::Location cl = Cover::Location::cover_location(md);

			bool success = Tagging::Covers::write_cover(md.filepath(), pm);
			if(!success)
			{
				sp_log(Log::Warning, this) << "Failed to write cover";
			}

			pm.save(cl.audio_file_target());

			db_covers->set_cover(cl.hash(), pm);

			progress++;
		}
	}

	db->commit();
	db->close_db();

	db->library_connector()->create_indexes();

	Cover::ChangeNotfier::instance()->shout();
	ChangeNotifier::instance()->change_metadata(m->v_md_before_change, m->v_md_after_change);

	emit sig_finished();
}

QMap<QString, Editor::FailReason> Editor::failed_files() const
{
	return m->failed_files;
}
