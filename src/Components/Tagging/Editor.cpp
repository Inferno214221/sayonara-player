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

#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverChangeNotifier.h"
#include "Components/Directories/MetaDataScanner.h"
#include "Components/MetaDataInfo/MetaDataInfo.h"

#include "Database/Connector.h"
#include "Database/CoverConnector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Library/Filter.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Tagging/TaggingCover.h"

#include <QHash>
#include <QFileInfo>

using Tagging::Editor;

namespace
{
	QList<AlbumId> getOriginalAlbumIds(const QList<Tagging::ChangeInformation>& changeInfos)
	{
		Util::Set<AlbumId> albumIds;
		for(const auto& changeInfo : changeInfos)
		{
			albumIds << changeInfo.originalMetadata().albumId();
		}

		return albumIds.toList();
	}

	Editor::FailReason checkFailReason(const QString& filepath, Editor* editor)
	{
		if(const auto fileInfo = QFileInfo(filepath); !fileInfo.exists())
		{
			spLog(Log::Warning, editor) << "Failed to write tags to file: File not found: " << filepath;
			return Editor::FailReason::FileNotFound;
		}

		else if(!fileInfo.isWritable())
		{
			spLog(Log::Warning, editor) << "Failed to write tags to file: File not writeable: " << filepath;
			return Editor::FailReason::FileNotWriteable;
		}

		spLog(Log::Warning, editor) << "Failed to write tags to file: Other error: " << filepath;
		return Editor::FailReason::TagLibError;
	}

	QPixmap scalePixmap(const QPixmap& pixmap)
	{
		constexpr const auto MaxSize = 1000;
		return ((pixmap.size().width() > MaxSize) || (pixmap.size().height() > MaxSize))
		       ? pixmap.scaled(QSize(MaxSize, MaxSize), Qt::KeepAspectRatio, Qt::SmoothTransformation)
		       : pixmap;
	}

	bool saveCoverInTrack(const QString& filepath, const QPixmap& pixmap, Editor* editor)
	{
		const auto success = Tagging::writeCover(filepath, pixmap);
		if(!success)
		{
			spLog(Log::Warning, editor) << "Failed to write cover";
		}

		return success;
	}

	bool saveCoverToPersistence(const MetaData& track, const QPixmap& cover, DB::Covers* coverDatabase, Editor* editor)
	{
		const auto coverLocation = Cover::Location::coverLocation(track);

		const auto pixmap = scalePixmap(cover);
		saveCoverInTrack(track.filepath(), pixmap, editor);

		pixmap.save(coverLocation.audioFileTarget());

		return coverDatabase->setCover(coverLocation.hash(), pixmap);
	}

	bool applyTrackChangesToDatabase(const MetaData& currentMetadata, DB::LibraryDatabase* libraryDatabase)
	{
		return (!currentMetadata.isExtern() && (currentMetadata.id() >= 0))
		       ? libraryDatabase->updateTrack(currentMetadata)
		       : true;
	}

	Editor::FailReason applyTrackChangesToFile(const MetaData& currentMetadata, Editor* editor)
	{
		return Tagging::Utils::setMetaDataOfFile(currentMetadata)
		       ? Editor::FailReason::NoError
		       : checkFailReason(currentMetadata.filepath(), editor);
	}

	int checkForChanges(const QList<Tagging::ChangeInformation>& changeInformation)
	{
		return Util::Algorithm::count(changeInformation, [](const auto& changeInfo) {
			return (changeInfo.hasChanges() || changeInfo.hasNewCover());
		});
	}
}

struct Editor::Private
{
	QList<ChangeInformation> changeInfo;
	QMap<QString, Editor::FailReason> failedFiles;
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

void Editor::updateTrack(int index, const MetaData& track)
{
	if(Util::between(index, m->changeInfo))
	{
		m->changeInfo[index].update(track);
	}
}

void Editor::undo(int index)
{
	if(Util::between(index, m->changeInfo))
	{
		m->changeInfo[index].undo();
	}
}

void Editor::undoAll()
{
	for(auto& changeInfo : m->changeInfo)
	{
		changeInfo.undo();
	}
}

MetaData Editor::metadata(int index) const
{
	return (Util::between(index, m->changeInfo))
	       ? m->changeInfo[index].currentMetadata()
	       : MetaData();
}

MetaDataList Editor::metadata() const
{
	MetaDataList tracks;

	Util::Algorithm::transform(m->changeInfo, tracks, [](const auto& changeInfo) {
		return (changeInfo.currentMetadata());
	});

	return tracks;
}

bool Editor::applyRegularExpression(const QString& regex, int index)
{
	if(!Util::between(index, m->changeInfo))
	{
		return false;
	}

	auto& changeInfo = m->changeInfo[index];
	auto& currentMetadata = changeInfo.currentMetadata();
	const auto expression = Tagging::Expression(regex, currentMetadata.filepath());

	if(expression.isValid())
	{
		const auto success = expression.apply(currentMetadata);
		changeInfo.setChanged(success);
	}

	return expression.isValid();
}

int Editor::count() const
{
	return m->changeInfo.count();
}

bool Editor::hasChanges() const
{
	return Util::Algorithm::contains(m->changeInfo, [](const auto& changeInfo) {
		return changeInfo.hasChanges();
	});
}

void Editor::addGenre(int index, const Genre& genre)
{
	if(Util::between(index, m->changeInfo))
	{
		auto& currentMetadata = m->changeInfo[index].currentMetadata();
		if(currentMetadata.addGenre(genre))
		{
			m->changeInfo[index].setChanged(true);
		}
	}
}

void Editor::deleteGenre(int index, const Genre& genre)
{
	if(Util::between(index, m->changeInfo))
	{
		auto& currentMetadata = m->changeInfo[index].currentMetadata();
		if(currentMetadata.removeGenre(genre))
		{
			m->changeInfo[index].setChanged(true);
		}
	}
}

void Editor::renameGenre(int index, const Genre& genre, const Genre& newGenre)
{
	deleteGenre(index, genre);
	addGenre(index, newGenre);
}

void Editor::setMetadata(const MetaDataList& tracks)
{
	m->failedFiles.clear();
	m->changeInfo.clear();

	Util::Algorithm::transform(tracks, m->changeInfo, [](const auto& track) {
		return ChangeInformation(track);
	});

	emit sigMetadataReceived(tracks);
}

bool Editor::isCoverSupported(int index) const
{
	if(Util::between(index, m->changeInfo))
	{
		const auto& originalMetadata = m->changeInfo[index].originalMetadata();
		return Tagging::isCoverSupported(originalMetadata.filepath());
	}

	return false;
}

bool Editor::canLoadEntireAlbum() const
{
	const auto albumIds = getOriginalAlbumIds(m->changeInfo);
	return (albumIds.count() == 1);
}

void Editor::loadEntireAlbum()
{
	const auto albumIds = getOriginalAlbumIds(m->changeInfo);
	if(albumIds.size() != 1)
	{
		return;
	}

	const auto albumId = albumIds.first();
	if((albumId < 0) && (!m->changeInfo.isEmpty()))
	{
		const auto filepath = m->changeInfo[0].originalMetadata().filepath();
		startSameAlbumCrawler(filepath);

		emit sigStarted();
		emit sigProgress(-1);
	}

	else
	{
		auto* libraryDatabase = DB::Connector::instance()->libraryDatabase(-1, 0);

		MetaDataList tracks;
		libraryDatabase->getAllTracksByAlbum(IdList {albumId}, tracks, ::Library::Filter(), -1);
		tracks.sort(::Library::SortOrder::TrackDiscnumberAsc);
		setMetadata(tracks);
	}
}

void Editor::startSameAlbumCrawler(const QString& filepath)
{
	using Directory::MetaDataScanner;

	const auto[dir, filename] = Util::File::splitFilename(filepath);

	auto* thread = new QThread();
	auto* worker = new MetaDataScanner({dir}, true);
	worker->moveToThread(thread);

	connect(thread, &QThread::finished, thread, &QObject::deleteLater);
	connect(thread, &QThread::started, worker, &MetaDataScanner::start);
	connect(worker, &MetaDataScanner::sigFinished, thread, &QThread::quit);
	connect(worker, &MetaDataScanner::sigFinished, this, &Editor::loadEntireAlbumFinished);

	thread->start();
}

void Editor::loadEntireAlbumFinished()
{
	auto* worker = static_cast<Directory::MetaDataScanner*>(sender());

	if(const auto tracks = worker->metadata(); !tracks.isEmpty())
	{
		setMetadata(tracks);
	}

	worker->deleteLater();

	emit sigFinished();
}

void Editor::insertMissingArtistsAndAlbums()
{
	MetaDataList tracksToBeModified;
	Util::Algorithm::transform(m->changeInfo, tracksToBeModified, [](const auto& changeInfo) {
		return changeInfo.currentMetadata();
	});

	auto* libraryDatabase = DB::Connector::instance()->libraryDatabase(-1, 0);
	const auto modifiedTracks = libraryDatabase->insertMissingArtistsAndAlbums(tracksToBeModified);

	auto i = 0;
	for(auto it = m->changeInfo.begin(); it != m->changeInfo.end(); it++, i++)
	{
		if(!Util::between(i, modifiedTracks))
		{
			spLog(Log::Warning, this) << "Index out of bounds when updating tracks!";
			break;
		}

		it->update(modifiedTracks[i]);
	}
}

void Editor::updateCover(int index, const QPixmap& cover)
{
	if(isCoverSupported(index) && !cover.isNull())
	{
		m->changeInfo[index].updateCover(cover);
	}
}

bool Editor::hasCoverReplacement(int index) const
{
	return (Util::between(index, m->changeInfo) && m->changeInfo[index].hasNewCover());
}

struct CommitResult
{
	bool coverChanged {false};
	QList<MetaDataPair> changedTracks;
	QMap<QString, Editor::FailReason> failedFiles;
};

void Editor::commit()
{
	const auto numChanges = checkForChanges(m->changeInfo);
	if(numChanges == 0)
	{
		return;
	}

	emit sigStarted();

	insertMissingArtistsAndAlbums();

	auto* db = DB::Connector::instance();
	auto* libraryDatabase = db->libraryDatabase(-1, 0);
	auto* coverDatabase = db->coverConnector();

	db->transaction();

	auto progress = 0;
	auto commitResult = CommitResult();

	for(auto& changeInfo : m->changeInfo)
	{
		const auto& currentMetadata = changeInfo.currentMetadata();
		const auto& originalMetadata = changeInfo.originalMetadata();

		if(changeInfo.hasNewCover())
		{
			commitResult.coverChanged |=
				saveCoverToPersistence(currentMetadata, changeInfo.cover(), coverDatabase, this);
		}

		if(changeInfo.hasChanges())
		{
			const auto writeResult = applyTrackChangesToFile(currentMetadata, this);

			if((writeResult == Editor::FailReason::NoError) &&
			   applyTrackChangesToDatabase(currentMetadata, libraryDatabase))
			{
				commitResult.changedTracks << MetaDataPair(originalMetadata, currentMetadata);
				changeInfo.apply();
			}

			else
			{
				commitResult.failedFiles.insert(originalMetadata.filepath(), writeResult);
				changeInfo.undo();
			}
		}

		emit sigProgress((++progress * 100) / numChanges);
	}

	db->commit();
	db->libraryConnector()->createIndexes();
	db->closeDatabase();

	m->failedFiles = std::move(commitResult.failedFiles);

	if(commitResult.coverChanged)
	{
		Cover::ChangeNotfier::instance()->shout();
	}

	if(!commitResult.changedTracks.isEmpty())
	{
		Tagging::ChangeNotifier::instance()->changeMetadata(commitResult.changedTracks);
	}

	emit sigFinished();
}

QMap<QString, Editor::FailReason> Editor::failedFiles() const
{
	return m->failedFiles;
}
