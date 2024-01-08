/* UserTaggingOperations.cpp */

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

#include "UserTaggingOperations.h"
#include "Editor.h"
#include "ChangeNotifier.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Tagging/TagReader.h"
#include "Utils/Tagging/TagWriter.h"
#include "Utils/Utils.h"

#include <tuple>

using Tagging::UserOperations;
using Tagging::Editor;
using Tagging::TagWriterPtr;
using Tagging::TagReaderPtr;

struct RatingPair
{
	Rating oldRating {Rating::Last};
	Rating newRating {Rating::Last};
};

using TrackRatingHistory = QMap<TrackID, RatingPair>;

struct UserOperations::Private
{
	DB::LibraryDatabase* libraryDatabase = nullptr;
	TrackRatingHistory trackRatingHistory;
	TagReaderPtr tagReader;
	TagWriterPtr tagWriter;

	Private(TagReaderPtr tagReader, TagWriterPtr tagWriter, const LibraryId libraryId) :
		tagReader {std::move(tagReader)},
		tagWriter {std::move(tagWriter)}
	{
		auto* db = DB::Connector::instance();
		libraryDatabase = db->libraryDatabase(libraryId, db->databaseId());
	}
};

UserOperations::UserOperations(const TagReaderPtr& tagReader, const TagWriterPtr& tagWriter, const LibraryId libraryId,
                               QObject* parent) :
	QObject(parent),
	m {Pimpl::make<Private>(tagReader, tagWriter, libraryId)} {}

UserOperations::~UserOperations() = default;

Editor* UserOperations::createEditor()
{
	const bool useSelectiveTagging = GetSetting(Set::Tagging_UseSelectiveTagging);
	auto* editor = new Tagging::Editor(m->tagReader, m->tagWriter, useSelectiveTagging, nullptr);

	connect(editor, &Tagging::Editor::sigFinished, this, [this]() {
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

	for(int i = 0; i < tracks.count(); i++)
	{
		auto track = tracks[i];
		m->trackRatingHistory[track.id()] = {track.rating(), rating};

		track.setRating(rating);
		editor->updateTrack(i, track);
	}

	runEditor(editor);
}

void UserOperations::setAlbumRating(const Album& album, Rating rating)
{
	m->libraryDatabase->updateAlbumRating(album.id(), rating);

	auto newAlbum = album;
	newAlbum.setRating(rating);

	Tagging::ChangeNotifier::instance()->updateAlbums({AlbumPair(album, newAlbum)});
}

void UserOperations::mergeArtists(const Util::Set<Id>& artistIds, ArtistId targetArtist)
{
	if(artistIds.isEmpty())
	{
		return;
	}

	if(targetArtist < 0)
	{
		spLog(Log::Warning, this) << "Cannot merge artist: Target artist id < 0";
		return;
	}

	Artist artist;
	const auto success = m->libraryDatabase->getArtistByID(targetArtist, artist);
	if(!success)
	{
		return;
	}

	auto wrongIds = artistIds;
	wrongIds.remove(targetArtist);

	MetaDataList tracks;
	m->libraryDatabase->getAllTracksByArtist(wrongIds.toList(), tracks);

	auto* editor = createEditor();
	editor->setMetadata(tracks);

	const auto showAlbumArtists = GetSetting(Set::Lib_ShowAlbumArtists);
	for(int idx = 0; idx < tracks.count(); idx++)
	{
		auto& track = tracks[idx];
		if(showAlbumArtists)
		{
			track.setAlbumArtist(artist.name(), artist.id());
		}

		else
		{
			track.setArtistId(artist.id());
			track.setArtist(artist.name());
		}

		editor->updateTrack(idx, track);
	}

	runEditor(editor);

	for(auto it = artistIds.begin(); it != artistIds.end(); it++)
	{
		if(*it == targetArtist)
		{
			continue;
		}

		m->libraryDatabase->deleteArtist(*it);
	}
}

void UserOperations::mergeAlbums(const Util::Set<Id>& albumIds, AlbumId targetAlbum)
{
	if(albumIds.isEmpty())
	{
		return;
	}

	if(targetAlbum < 0)
	{
		spLog(Log::Warning, this) << "Cannot merge albums: Target album id < 0";
		return;
	}

	Album album;
	bool success = m->libraryDatabase->getAlbumByID(targetAlbum, album, true);
	if(!success)
	{
		return;
	}

	auto wrongIds = albumIds;
	wrongIds.remove(targetAlbum);

	MetaDataList tracks;
	m->libraryDatabase->getAllTracksByAlbum(wrongIds.toList(), tracks);

	auto* editor = createEditor();
	editor->setMetadata(tracks);

	for(auto idx = 0; idx < tracks.count(); idx++)
	{
		auto& track = tracks[idx];
		track.setAlbumId(album.id());
		track.setAlbum(album.name());

		editor->updateTrack(idx, track);
	}

	runEditor(editor);
}

void UserOperations::addGenre(Util::Set<Id> ids, const Genre& genre)
{
	MetaDataList tracks;
	m->libraryDatabase->getAllTracks(tracks);

	tracks.removeTracks([&](const MetaData& md) {
		return (!ids.contains(md.id()));
	});

	auto* editor = createEditor();
	editor->setMetadata(tracks);

	for(auto i = 0; i < tracks.count(); i++)
	{
		editor->addGenre(i, genre);
	}

	runEditor(editor);
}

void UserOperations::deleteGenres(const Util::Set<Genre>& genres)
{
	MetaDataList tracks;
	m->libraryDatabase->getAllTracks(tracks);

	Util::Algorithm::removeIf(tracks, [&](const auto& track) {
		return std::none_of(genres.begin(), genres.end(), [&track](const auto& genre) {
			return track.hasGenre(genre);
		});
	});

	auto* editor = createEditor();
	editor->setMetadata(tracks);

	for(int i = 0; i < tracks.count(); i++)
	{
		for(const auto& genre: genres)
		{
			editor->deleteGenre(i, genre);
		}
	}

	runEditor(editor);
}

void UserOperations::renameGenre(const Genre& genre, const Genre& newGenre)
{
	MetaDataList tracks;
	m->libraryDatabase->getAllTracks(tracks);

	tracks.removeTracks([&](const MetaData& md) {
		return (!md.hasGenre(genre));
	});

	auto* editor = createEditor();
	editor->setMetadata(tracks);

	for(auto i = 0; i < tracks.count(); i++)
	{
		editor->renameGenre(i, genre, newGenre);
	}

	runEditor(editor);
}

void UserOperations::applyGenreToMetadata(const MetaDataList& tracks, const Genre& genre)
{
	auto* editor = createEditor();
	editor->setMetadata(tracks);

	for(auto i = 0; i < tracks.count(); i++)
	{
		editor->addGenre(i, genre);
	}

	runEditor(editor);
}

Rating Tagging::UserOperations::newRating(TrackID trackId) const
{
	return m->trackRatingHistory[trackId].newRating;
}
