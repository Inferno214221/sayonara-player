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
#include "Components/Covers/Fetcher/CoverFetcherUrl.h"

#include "Utils/Algorithm.h"
#include "Utils/CoverUtils.h"
#include "Utils/FileUtils.h"
#include "Utils/Set.h"
#include "Utils/MetaData/MetaData.h"
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

namespace
{
	QStringList convertToStringList(const QList<QUrl>& urls)
	{
		QStringList result;
		Util::Algorithm::transform(urls, result, [](const auto& url) { return url.toString(); });
		return result;
	}

	QList<Url> convertDownloadUrls(const QStringList& urls)
	{
		auto* fetchManager = Cover::Fetcher::Manager::instance();

		QList<Url> result;
		Util::Algorithm::transform(urls, result, [&](const auto& url) {
			return Util::File::isImageFile(url)
			       ? fetchManager->directFetcherUrl(url)
			       : fetchManager->websiteFetcherUrl(url);
		});

		return result;
	}

	QString extractPathFromPathHint(const QString& pathHint)
	{
		const auto fileInfo = QFileInfo(pathHint);
		if(fileInfo.isFile())
		{
			return Util::File::getParentDirectory(pathHint);
		}

		return (fileInfo.isDir())
		       ? pathHint
		       : QString {};
	}

	QStringList extractLocalPathDirectories(const QStringList& localPathHints, int maxItems)
	{
		Util::Set<QString> result;

		for(const auto& pathHint: localPathHints)
		{
			const auto path = extractPathFromPathHint(pathHint);
			if(!path.isEmpty())
			{
				result << path;
				if(result.count() == maxItems)
				{
					break;
				}
			}
		}

		return QStringList(result.toList());;
	}

	bool checkAlbumForCoverHint(const Album& album)
	{
		return (album.customField("has-album-art").toInt() != 0) &&
		       !album.pathHint().isEmpty();
	}

	bool checkPathHintForCover(const QString& pathHint)
	{
		return Util::File::isFile(pathHint) && Tagging::hasCover(pathHint);
	}

	int checkPathHintsForCover(const QStringList& pathHints)
	{ // usually path hints are tracks
		return Util::Algorithm::indexOf(pathHints, [&](const auto& hint) {
			return checkPathHintForCover(hint);
		});
	}

	QStringList onlyKeepFirst(QStringList list)
	{
		return list.isEmpty() ? list : QStringList {list.first()};
	}

	QString getCoverHintForAlbum(const Album& album)
	{
		const auto hasCoverHint = checkAlbumForCoverHint(album);
		const auto pathHints = album.isSampler() ? album.pathHint() : onlyKeepFirst(album.pathHint());
		const auto index = hasCoverHint
			? (pathHints.size() - 1)
			: checkPathHintsForCover(pathHints);

		return (index >= 0)
			? pathHints[index]
			: QString{};
	}

	Album createAlbumFromTrack(const MetaData& track)
	{
		Album album;
		album.setId(track.albumId());
		album.setName(track.album());
		album.setArtists({track.artist()});
		album.setAlbumArtist(track.albumArtist());
		album.setDatabaseId(track.databaseId());
		album.setPathHint({track.filepath()});
		album.setCoverDownloadUrls(track.coverDownloadUrls());
		album.addCustomField("cover-hash", QString(), track.customField("cover-hash"));
		album.addCustomField("has-album-art", QString(), track.customField("has-album-art"));

		return album;
	}

	bool saveToAudioFileTarget(const QString& audioFileSource, const QString& audioFileTarget)
	{
		const auto pixmap = Tagging::extractCover(audioFileSource);
		return (!pixmap.isNull()) && pixmap.save(audioFileTarget);
	}
}

struct Location::Private
{
	QString searchTerm;
	QList<Url> searchUrls;
	QList<Url> searchTermUrls;
	QStringList localPathHints;
	QString identifier {"Invalid location"};
	QString audioFileSource;
	QString audioFileTarget;

	QString hash;

	bool freetextSearch {false};
	bool valid {false};

	Private() = default;
	~Private() = default;
	Private(const Private& other) = default;
	Private(Private&& other) = default;

	Private& operator=(const Private& other) = default;
	Private& operator=(Private&& other) = default;
};

Location::Location() :
	m {Pimpl::make<Location::Private>()}
{
	qRegisterMetaType<Location>("CoverLocation");
}

Location::Location(const Location& other) :
	m {Pimpl::make<Location::Private>(*(other.m))} {}

Location::Location(Location&& other) noexcept :
	m {Pimpl::make<Private>(std::move(*other.m))} {}

Location::~Location() = default;

Location& Location::operator=(const Location& other)
{
	*m = *(other.m);
	return *this;
}

Location& Location::operator=(Location&& other) noexcept
{
	*m = std::move(*other.m);
	return *this;
}

QString Location::invalidPath() { return ":/Icons/logo.png"; }

Location Location::invalidLocation() { return Location(); }

Location Location::coverLocation(const QString& albumName, const QString& artistName)
{
	if(albumName.trimmed().isEmpty() && artistName.trimmed().isEmpty())
	{
		return invalidLocation();
	}

	auto* fetchManager = Cover::Fetcher::Manager::instance();

	Location location;
	location.setValid(true);
	location.setHash(Util::Covers::calcCoverToken(artistName, albumName));
	location.setSearchTerm(QString("%1 %2").arg(artistName).arg(albumName));
	location.setSearchUrls(fetchManager->albumAddresses(artistName, albumName));
	location.setIdentifier(QString("CL:By album: %1 by %2").arg(albumName).arg(artistName));

	return location;
}

Location Location::coverLocation(const QString& albumName, const QStringList& artists)
{
	return coverLocation(albumName, ArtistList::majorArtist(artists));
}

Location Location::coverLocation(const Album& album)
{
	auto location = (!album.albumArtist().trimmed().isEmpty())
	                ? Location::coverLocation(album.name(), album.albumArtist())
	                : Location::coverLocation(album.name(), album.artists());

	location.setLocalPathHints(album.pathHint());

	const auto coverHintPath = getCoverHintForAlbum(album);
	if(!coverHintPath.isEmpty())
	{
		if(!location.isValid())
		{ // poorly tagged, but path hints -> we need a hash
			const auto token = Util::Covers::calcCoverToken(coverHintPath, {});
			location.setHash(QString("poorly-tagged-album-%1").arg(token));
		}

		location.setValid(true);
		location.setAudioFileSource(coverHintPath, location.hashPath());
	}

	const auto searchUrls = convertDownloadUrls(album.coverDownloadUrls()) + location.searchUrls();
	location.setSearchUrls(searchUrls);
	if(!location.isValid() && !searchUrls.isEmpty())
	{ // poorly tagged, no path hints, but search urls -> we need a hash
		const auto token = Util::Covers::calcCoverToken(searchUrls.first().url(), {});
		location.setHash(QString("only-search-urls-available-%1").arg(token));
		location.setValid(true);
	}

	return location;
}

Location Location::coverLocation(const QString& artist)
{
	if(artist.trimmed().isEmpty())
	{
		return invalidLocation();
	}

	auto* fetchManager = Cover::Fetcher::Manager::instance();

	Location location;
	location.setValid(true);
	location.setHash(QString("artist_%1").arg(Util::Covers::calcCoverToken(artist, "")));
	location.setSearchUrls(fetchManager->artistAddresses(artist));
	location.setSearchTerm(artist);
	location.setIdentifier(QString("CL:By artist: %1").arg(artist));

	return location;
}

Location Location::coverLocation(const Artist& artist)
{
	auto location = Location::coverLocation(artist.name());
	if(location.isValid())
	{
		const auto searchUrls = convertDownloadUrls(artist.coverDownloadUrls()) + location.searchUrls();
		location.setSearchUrls(searchUrls);
	}

	return location;
}

Location Location::coverLocationRadio(const QString& stationName, const QString& stationUrl,
                                      const QStringList& coverDownloadUrls)
{
	if(stationName.trimmed().isEmpty())
	{
		return invalidLocation();
	}

	auto* fetchManager = Cover::Fetcher::Manager::instance();

	const auto urls =
		convertDownloadUrls(coverDownloadUrls) +
		fetchManager->radioSearchAddresses(stationName, stationUrl);

	Location location;
	location.setValid(true);
	location.setHash(QString("radio_%1").arg(Util::Covers::calcCoverToken(stationName, "")));
	location.setSearchUrls(urls);
	location.setSearchTerm(stationName);
	location.setIdentifier(QString("CL:By radio station: %1").arg(stationName));

	return location;
}

Location Location::coverLocation(const MetaData& track)
{
	return (track.radioMode() == RadioMode::Station)
	       ? coverLocationRadio(track.radioStationName(), track.filepath(), track.coverDownloadUrls())
	       : Location::coverLocation(createAlbumFromTrack(track));
}

Location Location::coverLocation(const QList<QUrl>& qtUrls, const QString& token)
{
	const auto urls = convertToStringList(qtUrls);

	Location location;
	location.setValid(true);
	location.setHash(token);
	location.setSearchUrls(convertDownloadUrls(urls));
	location.setIdentifier(QString("CL:By direct download url: %1").arg(urls.join(';')));

	return location;
}

QString Location::preferredPath() const
{
	if(isValid() && hasAudioFileSource())
	{
		if(Util::File::exists(audioFileTarget()) ||
		   saveToAudioFileTarget(audioFileSource(), audioFileTarget()))
		{
			return audioFileTarget();
		}
	}

	const auto localPath = this->localPath();
	return (isValid() && !localPath.isEmpty())
	       ? localPath
	       : invalidPath();
}

bool Location::isValid() const { return m->valid; }

void Location::setValid(bool valid) { m->valid = valid; }

QString Location::hash() const { return m->hash; }

void Location::setHash(const QString& hash) { m->hash = hash; }

QString Location::hashPath() const
{
	return isValid()
	       ? Util::coverDirectory(m->hash)
	       : QString {};
}

QString Location::identifier() const { return m->identifier; }

void Location::setIdentifier(const QString& identifier) { m->identifier = identifier; }

QString Location::alternativePath() const
{
	return (isValid())
	       ? QString("%1/alt_%2.png")
		       .arg(Util::coverDirectory())
		       .arg(m->hash)
	       : QString {};
}

QString Location::searchTerm() const { return m->searchTerm; }

void Location::setSearchTerm(const QString& searchTerm, const QString& coverFetcherIdentifier)
{
	auto* fetchManager = Cover::Fetcher::Manager::instance();

	m->searchTerm = searchTerm;
	m->searchTermUrls = (coverFetcherIdentifier.isEmpty())
	                    ? fetchManager->searchAddresses(searchTerm)
	                    : fetchManager->searchAddresses(searchTerm, coverFetcherIdentifier);
}

bool Location::hasSearchUrls() const { return !(m->searchUrls.isEmpty()); }

QList<Url> Location::searchUrls() const
{
	return (m->freetextSearch)
	       ? m->searchTermUrls
	       : m->searchUrls;
}

void Location::setSearchUrls(const QList<Url>& urls) { m->searchUrls = urls; }

void Location::enableFreetextSearch(bool enabled) { m->freetextSearch = enabled; }

QString Location::audioFileSource() const { return m->audioFileSource; }

QString Location::audioFileTarget() const { return m->audioFileTarget; }

bool Location::hasAudioFileSource() const
{
	return ((!m->audioFileTarget.isEmpty()) &&
	        (!m->audioFileSource.isEmpty()) &&
	        (Util::File::exists(m->audioFileSource)));
}

bool Location::setAudioFileSource(const QString& audioFilepath, const QString& coverPath)
{
	m->audioFileSource.clear();
	m->audioFileTarget.clear();

	if(coverPath.isEmpty() || !Util::File::isFile(audioFilepath))
	{
		return false;
	}

	auto[dir, filename] = Util::File::splitFilename(coverPath);
	const auto extension = Util::File::getFileExtension(coverPath);
	if(extension.isEmpty())
	{
		filename += QStringLiteral(".png");
	}

	m->audioFileSource = audioFilepath;
	m->audioFileTarget = QString("%1/fromtag_%2").arg(dir).arg(filename);

	return true;
}

QString Location::localPath() const
{
	if(Util::File::isFile(hashPath()))
	{
		return hashPath();
	}

	const auto localDir = localPathDir();
	if(Util::File::exists(localDir))
	{
		const auto localPaths = Cover::LocalSearcher::coverPathsFromPathHint(localDir);
		if(!localPaths.isEmpty())
		{
			Util::File::createSymlink(localPaths.first(), hashPath());
			return hashPath();
		}
	}

	return QString {};
}

QString Location::localPathDir() const
{
	const auto parentDirectories = extractLocalPathDirectories(localPathHints(), 1);
	return (!parentDirectories.isEmpty())
	       ? parentDirectories.first()
	       : QString {};
}

QStringList Location::localPathHints() const { return m->localPathHints; }

void Location::setLocalPathHints(const QStringList& pathHints)
{
	m->localPathHints.clear();
	Util::Algorithm::copyIf(pathHints, m->localPathHints, [](const auto& pathHint) {
		return (!Util::File::isWWW(pathHint));
	});
}
