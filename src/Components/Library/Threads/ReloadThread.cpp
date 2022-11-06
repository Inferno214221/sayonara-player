/* ReloadThread.cpp */

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
 * ReloadThread.cpp
 *
 *  Created on: Jun 19, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "ReloadThread.h"
#include "ReloadThreadFileScanner.h"

#include "Components/Covers/CoverLocation.h"
#include "Database/Connector.h"
#include "Database/CoverConnector.h"
#include "Database/Library.h"
#include "Database/LibraryDatabase.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Tagging/TagReader.h"
#include "Utils/Utils.h"

#include <QDir>

#include <utility>

namespace
{
	using PathTrackMap = QHash<QString, MetaData>;
	using Library::ReloadThread;
	struct LibraryAnalytics
	{
		PathTrackMap trackMap;
		MetaDataList orphanedTracks;
	};

	constexpr const auto NumFilesToStoreAtOnce = 500;
	constexpr const auto ClassName = "Library::ReloadThread";

	using OnProgressChanged = std::function<void(int, int, QString)>; // index, count, message

	bool compareGenres(const Util::Set<GenreID>& genres1, const Util::Set<GenreID>& genres2)
	{
		return std::equal(genres1.begin(), genres1.end(), genres2.begin(), genres2.end());
	}

	bool tracksAreEqual(const MetaData& track1, const MetaData& track2)
	{
		return (compareGenres(track1.genreIds(), track2.genreIds())) &&
		       (track1.title() == track2.title()) &&
		       (track1.album() == track2.album()) &&
		       (track1.artist() == track2.artist()) &&
		       (track1.year() == track2.year()) &&
		       (track1.rating() == track2.rating()) &&
		       (track1.discnumber() == track2.discnumber()) &&
		       (track1.trackNumber() == track2.trackNumber()) &&
		       (track1.albumArtist() == track2.albumArtist()) &&
		       (track1.albumArtistId() == track2.albumArtistId());
	}

	LibraryAnalytics analyzeLibrary(DB::LibraryDatabase* libraryDatabase, Library::ReloadThreadFileScanner* fileScanner)
	{
		auto tracks = MetaDataList {};
		libraryDatabase->getAllTracks(tracks);

		auto libraryAnalytics = LibraryAnalytics {};
		for(auto& track: tracks)
		{
			if(!fileScanner->checkFile(track.filepath()))
			{
				libraryAnalytics.orphanedTracks.push_back(std::move(track));
			}

			else
			{
				libraryAnalytics.trackMap[track.filepath()] = track;
			}
		}

		return libraryAnalytics;
	}

	MetaData tryReadTrack(const QString& filepath, const LibraryId libraryId, const Tagging::TagReaderPtr& tagReader)
	{
		if(auto track = tagReader->readMetadata(filepath); track.has_value())
		{
			track->setLibraryid(libraryId);
			return track.value();
		}

		return {};
	}

	MetaData checkLibraryTrack(const MetaData& track, const LibraryId libraryId, const Library::ReloadQuality quality,
	                           const Tagging::TagReaderPtr& tagReader)
	{
		assert(track.libraryId() >= 0);
		if(quality == Library::ReloadQuality::Fast)
		{
			return {};
		}

		const auto metadata = tryReadTrack(track.filepath(), libraryId, tagReader);
		return !tracksAreEqual(metadata, track) ? metadata : MetaData {};
	}

	void deleteDoubleTracks(DB::LibraryDatabase* libraryDatabase, const QString& libraryPath, const bool& mayRun)
	{
		auto tracksToUpdate = MetaDataList {};
		libraryDatabase->deleteInvalidTracks(libraryPath, tracksToUpdate);
		if(mayRun)
		{
			libraryDatabase->storeMetadata(tracksToUpdate);
		}
	}

	QString storeCover(const MetaData& track, const Util::Set<QString>& allHashes, DB::Covers* coverDatabase)
	{
		const auto coverLocation = Cover::Location::coverLocation(track);
		if(!coverLocation.isValid())
		{
			return {};
		}

		auto hash = coverLocation.hash(); // not const because of automatic move on return
		if(!hash.isEmpty() && !allHashes.contains(hash))
		{
			const auto preferredPath = coverLocation.preferredPath();
			if(QFile::exists(preferredPath))
			{
				const auto cover = QPixmap {preferredPath};
				if(!cover.isNull())
				{
					coverDatabase->insertCover(hash, cover);
					return hash;
				}
			}
		}

		return {};
	}

	void storeCovers(const MetaDataList& tracks)
	{
		auto* db = DB::Connector::instance();
		db->transaction();

		auto* coverDatabase = db->coverConnector();
		auto allHashes = coverDatabase->getAllHashes();
		for(const auto& track: tracks)
		{
			const auto hash = storeCover(track, allHashes, coverDatabase);
			if(!hash.isEmpty())
			{
				allHashes.insert(hash);
			}
		}

		db->commit();
	}

	void finalizeReloading(DB::Library* dbLibrary)
	{
		spLog(Log::Develop, ClassName) << "Updating album artists... ";
		dbLibrary->addAlbumArtists();

		spLog(Log::Develop, ClassName) << "Create indexes... ";
		dbLibrary->createIndexes();

		spLog(Log::Develop, ClassName) << "Clean up... ";
	}

	void storeMetadataBlock(const MetaDataList& tracks, DB::LibraryDatabase* libraryDatabase)
	{
		libraryDatabase->storeMetadata(tracks);
		storeCovers(tracks);
	}

	MetaDataList appendScannedTrack(MetaData track, MetaDataList metadataBlock, DB::LibraryDatabase* libraryDatabase)
	{
		if(track.isValid())
		{
			metadataBlock << std::move(track);

			if(metadataBlock.size() >= NumFilesToStoreAtOnce)
			{
				storeMetadataBlock(metadataBlock, libraryDatabase);
				metadataBlock.clear();
			}
		}

		return metadataBlock;
	}

	void storeScannedFiles(const QStringList& files, const PathTrackMap& libraryTrackMap,
	                       DB::LibraryDatabase* libraryDatabase, const Library::ReloadQuality quality,
	                       const bool& mayRun, const Tagging::TagReaderPtr& tagReader,
	                       const OnProgressChanged& callback)
	{
		const auto libraryId = libraryDatabase->libraryId();
		auto tracksToStore = MetaDataList {};
		auto currentIndex = 0;
		for(const auto& filepath: files)
		{
			if(!mayRun)
			{
				return;
			}

			callback(currentIndex++, files.count(), filepath);

			auto trackToStore = libraryTrackMap.contains(filepath)
			                    ? checkLibraryTrack(libraryTrackMap[filepath], libraryId, quality, tagReader)
			                    : tryReadTrack(filepath, libraryId, tagReader);

			tracksToStore = appendScannedTrack(std::move(trackToStore), std::move(tracksToStore), libraryDatabase);
		}

		storeMetadataBlock(tracksToStore, libraryDatabase);
	}
}

namespace Library
{
	struct ReloadThread::Private
	{
		ReloadThreadFileScanner* fileScanner;
		Tagging::TagReaderPtr tagReader;
		QString libraryPath {GetSetting(Set::Lib_Path)};
		ReloadQuality quality {ReloadQuality::Fast};
		LibraryId libraryId {-1};
		bool mayRun {true};

		explicit Private(ReloadThreadFileScanner* fileScanner, Tagging::TagReaderPtr tagReader) :
			fileScanner {fileScanner},
			tagReader {std::move(tagReader)} {}

		~Private()
		{
			delete fileScanner;
		}
	};

	ReloadThread::ReloadThread(ReloadThreadFileScanner* fileScanner, const Tagging::TagReaderPtr& tagReader,
	                           QObject* parent) :
		QThread(parent),
		m {Pimpl::make<Private>(fileScanner, tagReader)}
	{
		connect(m->fileScanner, &ReloadThreadFileScanner::sigCurrentDirectoryChanged, this,
		        [&](const auto& currentDir) { emit sigReloadingLibrary(currentDir, 0); });
	}

	ReloadThread::~ReloadThread()
	{
		stop();
		while(isRunning())
		{
			constexpr const auto ShutdownTimeout = 50;
			::Util::sleepMs(ShutdownTimeout);
		}
	}

	void ReloadThread::run()
	{
		if(m->libraryPath.isEmpty() || !m->fileScanner->exists(m->libraryPath))
		{
			spLog(Log::Warning, this) << "No valid library path given";
			return;
		}

		m->mayRun = true;

		auto* db = DB::Connector::instance();
		auto* libraryDatabase = db->libraryDatabase(m->libraryId, db->databaseId());

		emit sigReloadingLibrary(QObject::tr("Analyzing library") + "...", 0);
		const auto libraryAnalytics = analyzeLibrary(libraryDatabase, m->fileScanner);

		spLog(Log::Debug, ClassName) << libraryAnalytics.trackMap.count() << " tracks already in library";
		spLog(Log::Debug, ClassName) << libraryAnalytics.orphanedTracks.size() << " of them are not on disk anymore";

		if(m->mayRun)
		{
			cleanupLibrary(libraryAnalytics.orphanedTracks, libraryDatabase);
		}

		if(m->mayRun)
		{
			reloadLibrary(libraryAnalytics.trackMap, libraryDatabase);
		}

		finalizeReloading(db->libraryConnector());
	}

	void ReloadThread::cleanupLibrary(const MetaDataList& orphanedTracks, DB::LibraryDatabase* libraryDatabase)
	{
		emit sigReloadingLibrary(tr("Deleting double tracks") + "...", 0);
		deleteDoubleTracks(libraryDatabase, m->libraryPath, m->mayRun);
		if(m->mayRun)
		{
			emit sigReloadingLibrary(tr("Deleting orphaned tracks") + "...", 0);
			libraryDatabase->deleteTracks(Util::trackIds(orphanedTracks));
		}
	}

	bool ReloadThread::reloadLibrary(const PathTrackMap& pathTrackMap, DB::LibraryDatabase* libraryDatabase)
	{
		auto progressCallback = [&](const auto index, const auto count, const auto& message) {
			emit sigReloadingLibrary(message, (index * 100) / count); // NOLINT(readability-magic-numbers)
		};

		const auto files = m->fileScanner->getFilesRecursive({m->libraryPath});
		storeScannedFiles(files, pathTrackMap, libraryDatabase, m->quality, m->mayRun, m->tagReader, progressCallback);

		return true;
	}

	void ReloadThread::setLibrary(const LibraryId libraryId, const QString& libraryPath)
	{
		m->libraryPath = libraryPath;
		m->libraryId = libraryId;
	}

	void ReloadThread::stop() { m->mayRun = false; }

	void ReloadThread::setQuality(const Library::ReloadQuality quality) { m->quality = quality; }
}