/* UserTaggingOperations.cpp */

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

#include "UserTaggingOperations.h"
#include "Editor.h"
#include "ChangeNotifier.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/Album.h"
#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/Logger/Logger.h"

#include <tuple>

using Tagging::UserOperations;
using Tagging::Editor;

struct RatingPair
{
	Rating oldRating{Rating::Last};
	Rating newRating{Rating::Last};
};

using TrackRatingHistory = QMap<TrackID, RatingPair>;

struct UserOperations::Private
{
	DB::LibraryDatabase* libraryDatabase=nullptr;
	TrackRatingHistory trackRatingHistory;

	Private(LibraryId libraryId)
	{
		auto* db = DB::Connector::instance();
		libraryDatabase = db->libraryDatabase(libraryId, db->databaseId());
	}
};

UserOperations::UserOperations(LibraryId libraryId, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(libraryId);
}

UserOperations::~UserOperations() = default;

Editor* UserOperations::createEditor()
{
	auto* editor = new Tagging::Editor();

	connect(editor, &Tagging::Editor::sigFinished, this, [=](){
		m->trackRatingHistory.clear();
	});
	connect(editor, &Tagging::Editor::sigFinished, this, &UserOperations::sigFinished);
	connect(editor, &Tagging::Editor::sigProgress, this, &UserOperations::sigProgress);
	connect(editor, &Tagging::Editor::sigProgress, this, &UserOperations::sigProgress);
	connect(editor, &Tagging::Editor::sigProgress, this, &UserOperations::sigProgress);

	return editor;
}

void UserOperations::runEditor(Editor* editor)
{
	auto* t = new QThread();
	editor->moveToThread(t);

	connect(editor, &Tagging::Editor::sigFinished, t, &QThread::quit);
	connect(editor, &Tagging::Editor::sigFinished, editor, &QThread::deleteLater);

	connect(t, &QThread::started, editor, &Editor::commit);
	connect(t, &QThread::finished, t, &QObject::deleteLater);

	t->start();
}

void UserOperations::setTrackRating(const MetaData& md, Rating rating)
{
	m->trackRatingHistory[md.id()] = {md.rating(), rating};

	setTrackRating(MetaDataList(md), rating);
}

void UserOperations::setTrackRating(const MetaDataList& tracks, Rating rating)
{
	auto* editor = createEditor();
	editor->setMetadata(tracks);

	for(int i=0; i<tracks.count(); i++)
	{
		auto track = (tracks[i]);
		m->trackRatingHistory[track.id()] = {track.rating(), rating};

		track.setRating(rating);
		editor->updateTrack(i, track);
	}

	runEditor(editor);
}

void UserOperations::setAlbumRating(const Album& album, Rating rating)
{
	m->libraryDatabase->updateAlbumRating(album.id(), rating);

	Album newAlbum(album);
	newAlbum.setRating(rating);

	Tagging::ChangeNotifier::instance()->updateAlbums({AlbumPair(album, newAlbum)});
}

void UserOperations::mergeArtists(const Util::Set<Id>& artistIds, ArtistId targetArtist)
{
	if(artistIds.isEmpty()) {
		return;
	}

	if(targetArtist < 0){
		spLog(Log::Warning, this) << "Cannot merge artist: Target artist id < 0";
		return;
	}

	bool showAlbumArtists = GetSetting(Set::Lib_ShowAlbumArtists);

	Artist artist;
	bool success = m->libraryDatabase->getArtistByID(targetArtist, artist);
	if(!success){
		return;
	}

	Util::Set<ArtistId> wrongIds = artistIds;
	wrongIds.remove(targetArtist);

	MetaDataList tracks;
	m->libraryDatabase->getAllTracksByArtist(wrongIds.toList(), tracks);

	auto* editor = createEditor();
	editor->setMetadata(tracks);

	for(int idx=0; idx<tracks.count(); idx++)
	{
		MetaData md(tracks[idx]);
		if(showAlbumArtists){
			md.setAlbumArtist(artist.name(), artist.id());
		}

		else {
			md.setArtistId(artist.id());
			md.setArtist(artist.name());
		}

		editor->updateTrack(idx, md);
	}

	runEditor(editor);

	for(auto it = artistIds.begin(); it != artistIds.end(); it++)
	{
		if(*it == targetArtist){
			continue;
		}

		m->libraryDatabase->deleteArtist(*it);
	}
}

void UserOperations::mergeAlbums(const Util::Set<Id>& albumIds, AlbumId targetAlbum)
{
	if(albumIds.isEmpty())	{
		return;
	}

	if(targetAlbum < 0){
		spLog(Log::Warning, this) << "Cannot merge albums: Target album id < 0";
		return;
	}

	Album album;
	bool success = m->libraryDatabase->getAlbumByID(targetAlbum, album, true);
	if(!success) {
		return;
	}

	Util::Set<AlbumId> wrongIds = albumIds;
	wrongIds.remove(targetAlbum);

	MetaDataList tracks;
	m->libraryDatabase->getAllTracksByAlbum(wrongIds.toList(), tracks);

	auto* editor = createEditor();
	editor->setMetadata(tracks);

	for(int idx=0; idx<tracks.count(); idx++)
	{
		MetaData md(tracks[idx]);
		md.setAlbumId(album.id());
		md.setAlbum(album.name());

		editor->updateTrack(idx, md);
	}

	runEditor(editor);
}


void UserOperations::addGenre(Util::Set<Id> ids, const Genre& genre)
{
	MetaDataList tracks;
	m->libraryDatabase->getAllTracks(tracks);

	tracks.removeTracks([&ids](const MetaData& md) {
		return (!ids.contains(md.id()));
	});

	auto* editor = createEditor();
	editor->setMetadata(tracks);

	for(int i=0; i<tracks.count(); i++)
	{
		editor->addGenre(i, genre);
	}

	runEditor(editor);
}


void UserOperations::deleteGenre(const Genre& genre)
{
	MetaDataList tracks;
	m->libraryDatabase->getAllTracks(tracks);

	tracks.removeTracks([&genre](const MetaData& md){
		return (!md.hasGenre(genre));
	});

	auto* editor = createEditor();
	editor->setMetadata(tracks);

	for(int i=0; i<tracks.count(); i++)
	{
		editor->deleteGenre(i, genre);
	}

	runEditor(editor);
}

void UserOperations::renameGenre(const Genre& genre, const Genre& newGenre)
{
	MetaDataList tracks;
	m->libraryDatabase->getAllTracks(tracks);

	tracks.removeTracks([&genre](const MetaData& md){
		return (!md.hasGenre(genre));
	});

	auto* editor = createEditor();
	editor->setMetadata(tracks);

	for(int i=0; i<tracks.count(); i++)
	{
		editor->renameGenre(i, genre, newGenre);
	}

	runEditor(editor);
}

void UserOperations::applyGenreToMetadata(const MetaDataList& tracks, const Genre& genre)
{
	auto* editor = createEditor();
	editor->setMetadata(tracks);

	for(int i=0; i<tracks.count(); i++)
	{
		editor->addGenre(i, genre);
	}

	runEditor(editor);
}

Rating Tagging::UserOperations::oldRating(TrackID trackId) const
{
	return m->trackRatingHistory[trackId].oldRating;
}

Rating Tagging::UserOperations::newRating(TrackID trackId) const
{
	return m->trackRatingHistory[trackId].newRating;
}
