/* TagEdit.cpp */

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
	QList<ChangeInformation> changeInfo;

	// used for retrieving a list of all changed metadata
	MetaDataList			unchangedTracks;
	MetaDataList			changedTracks;

	QMap<QString, Editor::FailReason>	failedFiles;
};

Editor::Editor(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Editor::Private>();
}

Editor::Editor(const MetaDataList& tracks, QObject* parent) :
	Editor(parent)
{
	setMetadata(tracks);
}

Editor::~Editor() = default;

void Editor::updateTrack(int idx, const MetaData& md)
{
	if(Util::between(idx, m->changeInfo))
	{
		m->changeInfo[idx].update(md);
	}
}

void Editor::undo(int idx)
{
	if(Util::between(idx, m->changeInfo))
	{
		m->changeInfo[idx].undo();
	}
}

void Editor::undoAll()
{
	for(auto it=m->changeInfo.begin(); it != m->changeInfo.end(); it++)
	{
		it->undo();
	}
}

MetaData Editor::metadata(int idx) const
{
	if(Util::between(idx, m->changeInfo))
	{
		return m->changeInfo[idx].currentMetadata();
	}

	return MetaData();
}

MetaDataList Editor::metadata() const
{
	MetaDataList tracks;
	tracks.reserve( size_t(m->changeInfo.size()) );
	for(auto it=m->changeInfo.begin(); it != m->changeInfo.end(); it++)
	{
		tracks.push_back(it->currentMetadata());
	}

	return tracks;
}

bool Editor::applyRegularExpression(const QString& regex, int idx)
{
	if(!Util::between(idx, m->changeInfo)) {
		return false;
	}

	MetaData& md = m->changeInfo[idx].currentMetadata();
	Expression e(regex, md.filepath());
	if(!e.is_valid()) {
		return false;
	}

	bool b = e.apply(md);
	m->changeInfo[idx].setChanged(b);

	return true;
}

int Editor::count() const
{
	return m->changeInfo.count();
}

bool Editor::hasChanges() const
{
	return Algorithm::contains(m->changeInfo, [](const ChangeInformation& info){
		return info.hasChanges();
	});
}

void Editor::addGenre(int idx, const Genre& genre)
{
	if(!Util::between(idx, m->changeInfo))
	{
		MetaData& md = m->changeInfo[idx].currentMetadata();
		if(md.addGenre(genre))
		{
			m->changeInfo[idx].setChanged(true);
		}
	}
}

void Editor::deleteGenre(int idx, const Genre& genre)
{
	if(!Util::between(idx, m->changeInfo))
	{
		MetaData& md = m->changeInfo[idx].currentMetadata();
		if(md.removeGenre(genre))
		{
			m->changeInfo[idx].setChanged(true);
		}
	}
}

void Editor::renameGenre(int idx, const Genre& genre, const Genre& newGenre)
{
	deleteGenre(idx, genre);
	addGenre(idx, newGenre);
}


void Editor::setMetadata(const MetaDataList& tracks)
{
	m->changeInfo.clear();
	m->changeInfo.reserve(int(tracks.size()));

	for(const MetaData& md : tracks)
	{
		m->changeInfo << ChangeInformation(md);
	}

	m->failedFiles.clear();

	emit sigMetadataReceived(tracks);
}

bool Editor::isCoverSupported(int idx) const
{
	if(Util::between(idx, m->changeInfo))
	{
		const MetaData& md = m->changeInfo[idx].originalMetadata();
		return Tagging::Covers::isCoverSupported(md.filepath());
	}

	return false;
}

bool Editor::canLoadEntireAlbum() const
{	
	Util::Set<AlbumId> albumIds;

	for(const ChangeInformation& info : m->changeInfo)
	{
		albumIds << info.originalMetadata().albumId();
		if(albumIds.size() > 1) {
			return false;
		}
	}

	return (albumIds.size() == 1);
}

void Editor::loadEntireAlbum()
{
	Util::Set<AlbumId> albumIds;
	for(const ChangeInformation& info : m->changeInfo)
	{
		albumIds << info.originalMetadata().albumId();
	}

	if(albumIds.size() != 1){
		return;
	}

	AlbumId id = albumIds.first();
	if(id < 0 && m->changeInfo.size() > 0)
	{
		emit sigStarted();
		emit sigProgress(-1);

		QString dir, filename;
		QString path = m->changeInfo[0].originalMetadata().filepath();
		Util::File::splitFilename(path, dir, filename);

		using Directory::MetaDataScanner;
		auto* t = new QThread();
		auto* worker = new MetaDataScanner({dir}, true);
		worker->moveToThread(t);

		connect(t, &QThread::finished, t, &QObject::deleteLater);
		connect(t, &QThread::started, worker, &MetaDataScanner::start);
		connect(worker, &MetaDataScanner::sigFinished, t, &QThread::quit);
		connect(worker, &MetaDataScanner::sigFinished, this, &Editor::loadEntireAlbumFinished);

		t->start();
	}

	else
	{
		MetaDataList tracks;

		auto* db = DB::Connector::instance();
		auto* ldb = db->libraryDatabase(-1, 0);

		ldb->getAllTracksByAlbum(IdList{id}, tracks, ::Library::Filter(), -1);
		tracks.sort(::Library::SortOrder::TrackDiscnumberAsc);
		setMetadata(tracks);
	}
}

void Editor::loadEntireAlbumFinished()
{
	auto* worker = static_cast<Directory::MetaDataScanner*>(sender());

	MetaDataList tracks = worker->metadata();
	if(!tracks.isEmpty())
	{
		this->setMetadata(tracks);
	}

	worker->deleteLater();

	emit sigFinished();
}

void Editor::applyArtistsAndAlbumToMetadata()
{
	QHash<QString, ArtistId> artist_map;
	QHash<QString, AlbumId>	album_map;

	auto* db = DB::Connector::instance();
	auto* ldb = db->libraryDatabase(-1, 0);

	{ // load_all_albums
		AlbumList albums;
		ldb->getAllAlbums(albums, true);
		for(auto it=albums.begin(); it != albums.end(); it++)
		{
			if(album_map.contains(it->name()))
			{
				spLog(Log::Warning, this) << "Album " << it->name() << " already exists";
				continue;
			}

			album_map[it->name()] = it->id();
		}
	}

	{ // load all artists
		ArtistList artists;
		ldb->getAllArtists(artists, true);
		for(auto it=artists.begin(); it != artists.end(); it++)
		{
			if(artist_map.contains(it->name()))
			{
				spLog(Log::Warning, this) << "Artist " << it->name() << " already exists";
				continue;
			}

			artist_map[it->name()] = it->id();
		}
	}

	Util::Set<QString> insert_artists, insert_albums;
	{ // scan for unknown artists/albums
		for(auto it=m->changeInfo.begin(); it != m->changeInfo.end(); it++)
		{
			const MetaData& md = it->currentMetadata();
			if(!artist_map.contains(md.artist())) {
				insert_artists << md.artist();
			}

			if(!artist_map.contains(md.albumArtist())){
				insert_artists << md.albumArtist();
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
					spLog(Log::Warning, this) << "Invalid artist id";
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
					spLog(Log::Warning, this) << "Invalid album id";
				}
			}
		}
	}

	db->commit();

	for(auto it=m->changeInfo.begin(); it != m->changeInfo.end(); it++)
	{
		MetaData md = it->currentMetadata();

		md.setAlbumId( album_map[md.album()] );
		md.setArtistId( artist_map[md.artist()] );
		md.setAlbumArtistId( artist_map[md.albumArtist()] );

		it->update(md);
	}
}

void Editor::updateCover(int idx, const QPixmap& cover)
{
	if(isCoverSupported(idx))
	{
		m->changeInfo[idx].updateCover(cover);
	}
}

bool Editor::hasCoverReplacement(int idx) const
{
	return (Util::between(idx, m->changeInfo) && m->changeInfo[idx].hasNewCover());
}

void Editor::commit()
{
	emit sigStarted();

	auto* db = DB::Connector::instance();
	auto* db_covers = db->coverConnector();
	auto* ldb = db->libraryDatabase(-1, 0);

	m->unchangedTracks.clear();
	m->changedTracks.clear();

	const auto changed_tracks = std::count_if(m->changeInfo.begin(), m->changeInfo.end(), [](const ChangeInformation& info){
		return (info.hasChanges());
	});

	const auto changed_covers = std::count_if(m->changeInfo.begin(), m->changeInfo.end(), [](const ChangeInformation& info){
		return (info.hasNewCover());
	});

	if((changed_tracks + changed_covers) == 0)
	{
		emit sigFinished();
		return;
	}

	spLog(Log::Debug, this) << "Changing " << changed_tracks << " tracks and " << changed_covers << " covers";

	applyArtistsAndAlbumToMetadata();

	db->transaction();

	int progress = 0;
	for(auto it=m->changeInfo.begin(); it != m->changeInfo.end(); it++)
	{
		bool has_new_cover = it->hasNewCover();

		emit sigProgress(((progress + 1) * 100) / (changed_tracks + changed_covers));

		/* Normal tags changed */
		if(it->hasChanges())
		{
			bool success;
			{ // write Metadata to file
				success = Tagging::Utils::setMetaDataOfFile(it->currentMetadata());
				if(!success)
				{
					QString filepath = it->currentMetadata().filepath();
					QFileInfo fi(filepath);
					if(!fi.exists())
					{
						m->failedFiles.insert(filepath, FailReason::FileNotFound);
						spLog(Log::Warning, this) << "Failed to write tags to file: File not found: " << filepath;
					}

					else if(!fi.isWritable())
					{
						m->failedFiles.insert(filepath, FailReason::FileNotWriteable);
						spLog(Log::Warning, this) << "Failed to write tags to file: File not writeable: " << filepath;
					}

					else
					{
						m->failedFiles.insert(filepath, FailReason::TagLibError);
						spLog(Log::Warning, this) << "Failed to write tags to file: Other error: " << filepath;
					}
				}
			}

			if(success)
			{ // write changed to db
				const MetaData& org_md = it->originalMetadata();
				const MetaData& cur_md = it->currentMetadata();
				if( !cur_md.isExtern() && cur_md.id() >= 0 )
				{
					if(ldb->updateTrack(cur_md))
					{
						m->unchangedTracks << org_md;
						m->changedTracks << cur_md;

						// update track
						it->apply();
					}
				}

				else
				{
					m->unchangedTracks << org_md;
					m->changedTracks << cur_md;
				}
			}

			progress++;
		}

		/* Covers changed */
		if(has_new_cover)
		{
			QPixmap pm = it->cover();
			if(pm.size().width() > 1000 || pm.size().height() > 1000)
			{
				pm = pm.scaled(QSize(1000, 1000), Qt::KeepAspectRatio, Qt::SmoothTransformation);
			}

			const MetaData& md = it->currentMetadata();
			Cover::Location cl = Cover::Location::coverLocation(md);

			bool success = Tagging::Covers::writeCover(md.filepath(), pm);
			if(!success)
			{
				spLog(Log::Warning, this) << "Failed to write cover";
			}

			pm.save(cl.audioFileTarget());

			db_covers->setCover(cl.hash(), pm);

			progress++;
		}
	}

	db->commit();
	db->libraryConnector()->createIndexes();
	db->closeDatabase();

	Cover::ChangeNotfier::instance()->shout();
	ChangeNotifier::instance()->changeMetadata(m->unchangedTracks, m->changedTracks);

	emit sigFinished();
}

QMap<QString, Editor::FailReason> Editor::failedFiles() const
{
	return m->failedFiles;
}
