/* CoverLocation.cpp */

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

#include "CoverLocation.h"
#include "CoverFetchManager.h"
#include "LocalCoverSearcher.h"
#include "Fetcher/CoverFetcherUrl.h"

#include "Utils/Algorithm.h"
#include "Utils/CoverUtils.h"
#include "Utils/FileUtils.h"
#include "Utils/Set.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/StandardPaths.h"
#include "Utils/Tagging/TaggingCover.h"

#include <QDir>
#include <QUrl>
#include <QStringList>
#include <QPixmap>
#include <QFileInfo>

using Cover::Location;
using Cover::Fetcher::Url;

namespace{
	QList<Url> extractDownloadUrls(const LibraryItem* item)
	{
		const auto downloadUrls = item->coverDownloadUrls();
		auto* fetchManager = Cover::Fetcher::Manager::instance();

		QList<Url> urls;
		Util::Algorithm::transform(downloadUrls, urls, [&](const QString& downloadUrl) {
			return Util::File::isImageFile(downloadUrl)
			       ? fetchManager->directFetcherUrl(downloadUrl)
			       : fetchManager->websiteFetcherUrl(downloadUrl);
		});

		return urls;
	}
}

struct Location::Private
{
	QString searchTerm;
	QList<Url> searchUrls;
	QList<Url> searchTermUrls;
	QStringList localPathHints;
	QString identifier;
	QString audioFileSource;
	QString audioFileTarget;

	QString hash;

	bool freetextSearch;
	bool valid;

	Private() :
		freetextSearch(false),
		valid(false) {}

	~Private() = default;

	Private(const Private& other) = default;
	Private(Private&& other) = default;

	Private& operator=(const Private& other) = default;
	Private& operator=(Private&& other) = default;
};

Location::Location()
{
	qRegisterMetaType<Location>("CoverLocation");

	m = Pimpl::make<Location::Private>();
}

Location::~Location() = default;

Location::Location(const Location& other)
{
	m = Pimpl::make<Location::Private>(*(other.m));
}

Location::Location(Location&& other)
{
	m = Pimpl::make<Private>(std::move(*other.m));
}

Location& Location::operator=(const Location& other)
{
	*m = *(other.m);
	return *this;
}

Location& Location::operator=(Location&& other)
{
	*m = std::move(*other.m);
	return *this;
}

QString Location::invalidPath()
{
	return QString(":/Icons/logo.png");
}

Location Location::invalidLocation()
{
	Location location;

	location.setValid(false);
	location.setSearchUrls(QList<Url>());
	location.setSearchTerm(QString());
	location.setIdentifier("Invalid location");
	location.setAudioFileSource(QString(), QString());
	location.setLocalPathHints(QStringList());

	return location;
}

Location Location::coverLocation(const QString& albumName, const QString& artistName)
{
	if(albumName.trimmed().isEmpty() && artistName.trimmed().isEmpty())
	{
		return invalidLocation();
	}

	const auto coverToken = Util::Covers::calcCoverToken(artistName, albumName);
	auto* fetchManager = Cover::Fetcher::Manager::instance();

	Location location;
	{
		location.setValid(true);
		location.setHash(coverToken);
		location.setSearchTerm(artistName + " " + albumName);
		location.setSearchUrls(fetchManager->albumAddresses(artistName, albumName));
		location.setIdentifier("CL:By album: " + albumName + " by " + artistName);
	}

	return location;
}

Location Location::coverLocation(const QString& albumName, const QStringList& artists)
{
	const auto majorArtist = ArtistList::majorArtist(artists);
	return coverLocation(albumName, majorArtist);
}

Location Location::coverLocation(const Album& album)
{
	Location location;

	{ //setup basic CoverLocation
		if(!album.albumArtist().trimmed().isEmpty())
		{
			location = std::move(Location::coverLocation(album.name(), album.albumArtist()));
		}

		else if(album.artists().size() > 1)
		{
			location = std::move(Location::coverLocation(album.name(), album.artists()));
		}

		else if(!album.artists().isEmpty())
		{
			location = std::move(Location::coverLocation(album.name(), album.artists().first()));
		}

		else
		{
			location = std::move(Location::coverLocation(album.name(), QString()));
		}

		const auto urls = extractDownloadUrls(&album);
		if(!urls.isEmpty())
		{
			location.setSearchUrls(urls);
		}
	}

	// setup local paths. No audio file source. That may last too long
	{
		const auto pathHints = album.pathHint();
		if(!pathHints.isEmpty())
		{
			location.setLocalPathHints(pathHints);
			location.setAudioFileSource(pathHints.first(), location.hashPath());
		}
	}

	location.setSearchTerm(album.name() + " " + ArtistList::majorArtist(album.artists()));

	return location;
}

Location Location::coverLocation(const Artist& artist)
{
	auto location = std::move(Location::coverLocation(artist.name()));

	const auto urls = extractDownloadUrls(&artist);
	if(!urls.isEmpty())
	{
		location.setSearchUrls(urls);
	}

	location.setSearchTerm(artist.name());
	location.setIdentifier("CL:By artist: " + artist.name());

	return location;
}

Location Location::coverLocation(const QString& artist)
{
	if(artist.trimmed().isEmpty())
	{
		return invalidLocation();
	}

	const auto coverToken = QString("artist_%1").arg(Util::Covers::calcCoverToken(artist, ""));
	auto* fetchManager = Cover::Fetcher::Manager::instance();

	Location location;
	{
		location.setValid(true);
		location.setHash(coverToken);
		location.setSearchUrls(fetchManager->artistAddresses(artist));
		location.setSearchTerm(artist);
		location.setIdentifier("CL:By artist name: " + artist);
	}

	return location;
}

Location Location::coverLocationRadio(const QString& radioStation)
{
	if(radioStation.trimmed().isEmpty())
	{
		return invalidLocation();
	}

	const auto coverToken = QString("radio_%1").arg(Util::Covers::calcCoverToken(radioStation, ""));
	auto* fetchManager = Cover::Fetcher::Manager::instance();

	Location location;
	{
		location.setValid(true);
		location.setHash(coverToken);
		location.setSearchUrls(fetchManager->searchAddresses(radioStation));
		location.setSearchTerm(radioStation);
		location.setIdentifier("CL:By radio station: " + radioStation);
	}

	return location;
}

Location Location::coverLocation(const MetaData& track)
{
	return Location::coverLocation(track, true);
}

Location Location::coverLocation(const MetaData& track, bool checkForCoverart)
{
	Location location;

	const auto coverDownloadUrls = track.coverDownloadUrls();
	if(!coverDownloadUrls.isEmpty())
	{
		const auto coverToken = Util::Covers::calcCoverToken(track.artist(), track.album());

		QList<QUrl> urls;
		Util::Algorithm::transform(coverDownloadUrls, urls, [](const auto& url) {
			return QUrl(url);
		});

		location = std::move(Location::coverLocation(urls, coverToken));
	}

	else if(track.albumId() >= 0)
	{
		Album album;
		{
			album.setId(track.albumId());
			album.setName(track.album());
			album.setArtists({track.artist()});
			album.setAlbumArtist(track.albumArtist());
			album.setDatabaseId(track.databaseId());
			album.setPathHint({track.filepath()});
		}

		location = std::move(Location::coverLocation(album));
	}

	if(!location.isValid())
	{
		if(track.radioMode() == RadioMode::Station)
		{
			location = std::move(Location::coverLocationRadio(track.radioStation()));
		}

		else if(!track.album().isEmpty() && !track.artist().isEmpty())
		{
			location = std::move(Location::coverLocation(track.album(), track.artist()));
		}
	}

	const auto hasCoverArt = checkForCoverart
	                         ? Tagging::Covers::hasCover(track.filepath())
	                         : track.customField("has_album_art").toInt() != 0;

	if(location.audioFileSource().isEmpty() && !track.filepath().isEmpty() && hasCoverArt)
	{
		location.setAudioFileSource(track.filepath(), location.hashPath());
	}

	if(location.searchUrls().isEmpty())
	{
		const auto urls = extractDownloadUrls(&track);
		const auto identifier = QString("CL:By metadata: %1 by %2 with %3 direct download urls")
			.arg(track.album())
			.arg(track.artist())
			.arg(urls.size());

		location.setSearchUrls(urls);
		location.setIdentifier(identifier);
	}

	else
	{
		location.setIdentifier("CL:By metadata: " + track.album() + " by " + track.albumArtist());
	}

	location.setLocalPathHints(QStringList {track.filepath()});

	const auto customHash = track.customField("cover-hash");
	if(!customHash.isEmpty())
	{
		location.setHash(customHash);
	}

	return location;
}

Location Location::coverLocation(const QList<QUrl>& urls, const QString& token)
{
	QList<Url> fetchUrls;
	QStringList merged;
	for(const auto& url : urls)
	{
		merged << url.toString();
		auto* fetchManager = Cover::Fetcher::Manager::instance();
		const auto fetchUrl = (Util::File::isImageFile(url.toString())
		                       ? fetchManager->directFetcherUrl(url.toString())
		                       : fetchManager->websiteFetcherUrl(url.toString()));
		fetchUrls << fetchUrl;
	}

	Location location;
	{
		location.setValid(true);
		location.setHash(token);
		location.setSearchUrls(fetchUrls);
		location.setIdentifier("CL:By direct download url: " + merged.join(";"));
	}

	return location;
}

bool Location::isValid() const
{
	return m->valid;
}

QString Location::preferredPath() const
{
	if(!m->valid)
	{
		return Location::invalidPath();
	}

	// first search for cover in track
	if(hasAudioFileSource())
	{
		auto targetExists = Util::File::exists(this->audioFileTarget());
		if(!targetExists)
		{
			const auto pm = Tagging::Covers::extractCover(this->audioFileSource());
			if(!pm.isNull())
			{
				targetExists = pm.save(this->audioFileTarget());
			}
		}

		if(targetExists)
		{
			return audioFileTarget();
		}
	}

	return (!m->localPathHints.isEmpty())
	       ? localPath()
	       : Location::invalidPath();
}

QString Location::alternativePath() const
{
	const auto alternativePath = QString("%1/alt_%2.png")
		.arg(Util::coverDirectory())
		.arg(m->hash);

	return alternativePath;
}

void Location::setValid(bool valid)
{
	m->valid = valid;
}

void Location::setIdentifier(const QString& identifier)
{
	m->identifier = identifier;
}

QString Location::hashPath() const
{
	return Util::coverDirectory(m->hash);
}

QString Location::identifer() const
{
	return m->identifier;
}

QList<Url> Location::searchUrls() const
{
	return (m->freetextSearch)
	       ? m->searchTermUrls
	       : m->searchUrls;
}

bool Location::hasSearchUrls() const
{
	return !(m->searchUrls.isEmpty());
}

QString Location::searchTerm() const
{
	return m->searchTerm;
}

void Location::setSearchTerm(const QString& searchTerm)
{
	auto* fetchManager = Cover::Fetcher::Manager::instance();

	m->searchTerm = searchTerm;
	m->searchTermUrls = fetchManager->searchAddresses(searchTerm);
}

void Location::setSearchTerm(const QString& searchTerm, const QString& coverFetcherIdentifier)
{
	auto* fetchManager = Cover::Fetcher::Manager::instance();

	m->searchTerm = searchTerm;
	m->searchTermUrls = fetchManager->searchAddresses(searchTerm, coverFetcherIdentifier);
}

void Location::setSearchUrls(const QList<Url>& urls)
{
	m->searchUrls = urls;
}

void Location::enableFreetextSearch(bool enabled)
{
	m->freetextSearch = enabled;
}

bool Location::hasAudioFileSource() const
{
	return (
		(!m->audioFileTarget.isEmpty()) &&
		(!m->audioFileSource.isEmpty()) &&
		(Util::File::exists(m->audioFileSource)));
}

QString Location::audioFileSource() const
{
	return m->audioFileSource;
}

QString Location::audioFileTarget() const
{
	return m->audioFileTarget;
}

bool Location::setAudioFileSource(const QString& audioFilepath, const QString& coverPath)
{
	m->audioFileSource.clear();
	m->audioFileTarget.clear();

	if(audioFilepath.isEmpty() || coverPath.isEmpty())
	{
		return false;
	}

	auto[dir, filename] = Util::File::splitFilename(coverPath);

	const auto extension = Util::File::getFileExtension(filename);
	if(extension.isEmpty())
	{
		filename += ".png";
	}

	m->audioFileSource = audioFilepath;
	m->audioFileTarget = QString("%1/fromtag_%2").arg(dir).arg(filename);

	return true;
}

QString Location::localPath() const
{
	const auto localDir = localPathDir();
	if(localDir.isEmpty() || hashPath().isEmpty())
	{
		return QString();
	}

	const auto info = QFileInfo(hashPath());
	if(info.exists())
	{
		if(info.isSymLink())
		{
			const auto linkPath = hashPath();
			// delete broken link
			if(!Util::File::exists(info.symLinkTarget()))
			{
				Util::File::deleteFiles({linkPath});
			}

			else
			{ // symlink ok
				return linkPath;
			}
		}

		else if(info.isFile())
		{
			return hashPath();
		}

		else
		{
			spLog(Log::Warning, "CoverLocation") << "Cover path is no symlink and no regular file";
			return QString();
		}
	}

	const auto localPaths = Cover::LocalSearcher::coverPathsFromPathHint(localDir);
	if(localPaths.isEmpty())
	{
		return QString();
	}

	Util::File::createSymlink(localPaths.first(), hashPath());
	return hashPath();
}

QString Location::localPathDir() const
{
	Util::Set<QString> parentDirectories;

	const auto pathHints = localPathHints();
	for(const auto& localPath : pathHints)
	{
		const auto fileInfo = QFileInfo(localPath);
		if(!fileInfo.exists())
		{
			continue;
		}

		if(fileInfo.isFile())
		{
			parentDirectories << Util::File::getParentDirectory(localPath);
		}

		else if(fileInfo.isDir())
		{
			parentDirectories << localPath;
		}

		if(parentDirectories.size() > 1)
		{
			return QString {};
		}
	}

	return (parentDirectories.isEmpty())
	       ? QString()
	       : parentDirectories.first();
}

QStringList Location::localPathHints() const
{
	return m->localPathHints;
}

void Location::setLocalPathHints(const QStringList& pathHints)
{
	m->localPathHints.clear();
	Util::Algorithm::copyIf(pathHints, m->localPathHints, [](const auto& pathHint) {
		return (!Util::File::isWWW(pathHint));
	});
}

QString Location::hash() const
{
	return m->hash;
}

void Location::setHash(const QString& hash)
{
	m->hash = hash;
}

QString Location::toString() const
{
	return QString("Cover Location: Valid? %1, Preferred path: %2, Search Term: %3, Identifier: %4")
		.arg(m->valid)
		.arg(preferredPath())
		.arg(searchTerm())
		.arg(identifer());
}
