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
#include "CoverUtils.h"
#include "CoverFetchManager.h"
#include "LocalCoverSearcher.h"
#include "Fetcher/CoverFetcherUrl.h"

#include "Utils/globals.h"
#include "Utils/Set.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Tagging/TaggingCover.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QUrl>
#include <QMap>
#include <QStringList>
#include <QImage>
#include <QPixmap>
#include <QFileInfo>

using Cover::Location;
using namespace Cover::Fetcher;
using Cover::StringMap;

using FetchManager=Cover::Fetcher::Manager;
using Cover::Fetcher::Url;
using UrlList=QList<Url>;

namespace File=::Util::File;

static QList<Url> extract_downloadUrls(const LibraryItem* item)
{
	QList<Url> urls;
	const QStringList cdu = item->coverDownloadUrls();
	for(const QString& url : cdu)
	{
		urls << FetchManager::instance()->directFetcherUrl(url);
	}

	return urls;
}

struct Location::Private
{
	QString			searchTerm;		// Term provided to search engine
	UrlList			searchUrls;		// Search url where to fetch covers
	UrlList			searchTermUrls;	// Search urls where to fetch cover when using freetext search
	QStringList		localPathHints;
	QString			coverPath;			// coverPath path, in .Sayonara, where cover is stored. Ignored if localPaths are not empty
	QString			identifier;			// Some human readable identifier with methods where invokded
	QString			audioFileSource;	// A saved cover from an audio file
	QString			audioFileTarget;

	QString			hash;				// A unique identifier, mostly referred to as the cover token

	bool			freetextSearch;
	bool			valid;				// valid if CoverLocation object contains a valid download url

	Private() :
		freetextSearch(false),
		valid(false)
	{}

	~Private() = default;
	Private(const Private& other) = default;
	Private& operator=(const Private& other) = default;
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

Location& Location::operator=(const Location& other)
{
	*m = *(other.m);
	return *this;
}

QString Location::invalidPath()
{
	return QString(":/Icons/logo.png");
}

Location Location::invalidLocation()
{
	Location cl;

	cl.setValid(false);
	cl.setCoverPath(Location::invalidPath());
	cl.setSearchUrls(UrlList());
	cl.setSearchTerm(QString());
	cl.setIdentifier("Invalid location");
	cl.setAudioFileSource(QString(), QString());
	cl.setLocalPathHints(QStringList());

	return cl;
}

Location Location::coverLocation(const QString& albumName, const QString& artistName)
{
	using namespace Cover::Fetcher;
	if(albumName.trimmed().isEmpty() && artistName.trimmed().isEmpty())
	{
		return invalidLocation();
	}

	const QString coverToken = Cover::Utils::calcCoverToken(artistName, albumName);
	const QString coverPath = Cover::Utils::coverDirectory( coverToken + ".png" );
	auto* cfm = Fetcher::Manager::instance();

	Location ret;
	{
		ret.setValid(true);
		ret.setCoverPath(coverPath);
		ret.setHash(coverToken);
		ret.setSearchTerm(artistName + " " + albumName);
		ret.setSearchUrls(cfm->albumAddresses(artistName, albumName));
		ret.setIdentifier("CL:By album: " + albumName + " by " + artistName);
	}

	return ret;
}

Location Location::coverLocation(const QString& albumName, const QStringList& artists)
{
	const QString major_artist = ArtistList::majorArtist(artists);
	return coverLocation(albumName, major_artist);
}

Location Location::xcoverLocation(const Album& album)
{
	Location cl;

	{ //setup basic CoverLocation
		if(!album.albumArtist().trimmed().isEmpty())
		{
			cl = Location::coverLocation(album.name(), album.albumArtist());
		}

		else if(album.artists().size() > 1)
		{
			cl = Location::coverLocation(album.name(), album.artists());
		}

		else if(album.artists().size() == 1)
		{
			cl = Location::coverLocation(album.name(), album.artists().at(0));
		}

		else
		{
			cl = Location::coverLocation(album.name(), "");
		}

		const QList<Url> urls = extract_downloadUrls( &album );
		if(!urls.isEmpty()) {
			cl.setSearchUrls(urls);
		}
	}

	// setup local paths. No audio file source. That may last too long
	{
		const QStringList path_hints = album.pathHint();
		if(!path_hints.isEmpty())
		{
			cl.setLocalPathHints(path_hints);
			cl.setAudioFileSource(path_hints.first(), cl.coverPath());
		}
	}

	cl.setSearchTerm(album.name() + " " + ArtistList::majorArtist(album.artists()));

	return cl;
}

Location Location::coverLocation(const Artist& artist)
{
	Location cl = Location::coverLocation(artist.name());

	const QList<Url> urls = extract_downloadUrls(&artist);
	if(!urls.isEmpty()) {
		cl.setSearchUrls(urls);
	}

	cl.setSearchTerm(artist.name());
	cl.setIdentifier("CL:By artist: " + artist.name());

	return cl;
}

Location Location::coverLocation(const QString& artist)
{
	if(artist.trimmed().isEmpty()) {
		return invalidLocation();
	}

	const QString coverToken = QString("artist_") + Cover::Utils::calcCoverToken(artist, "");
	const QString coverPath = Cover::Utils::coverDirectory(coverToken + ".png");

	auto* cfm = Fetcher::Manager::instance();

	Location ret;
	{
		ret.setValid(true);
		ret.setCoverPath(coverPath);
		ret.setSearchUrls(cfm->artistAddresses(artist));
		ret.setSearchTerm(artist);
		ret.setIdentifier("CL:By artist name: " + artist);
		ret.setHash(coverToken);
	}

	return ret;
}

Location Location::coverLocationRadio(const QString& radioStation)
{
	if(radioStation.trimmed().isEmpty()) {
		return invalidLocation();
	}

	const QString coverToken = QString("radio_") + Cover::Utils::calcCoverToken(radioStation, "");
	const QString coverPath = Cover::Utils::coverDirectory(coverToken + ".png");

	auto* cfm = Fetcher::Manager::instance();

	Location ret;
	{
		ret.setValid(true);
		ret.setCoverPath(coverPath);
		ret.setSearchUrls(cfm->searchAddresses(radioStation));
		ret.setSearchTerm(radioStation);
		ret.setIdentifier("CL:By radio station: " + radioStation);
		ret.setHash(coverToken);
	}

	return ret;
}


Location Location::coverLocation(const MetaData& md)
{
	return Location::coverLocation(md, true);
}

Location Location::coverLocation(const MetaData& md, bool checkForCoverart)
{
	Location cl;

	const QStringList cdu = md.coverDownloadUrls();
	if(!cdu.isEmpty())
	{
		const QString extension = File::getFileExtension(cdu.first());
		const QString coverToken = Cover::Utils::calcCoverToken(md.artist(), md.album());
		const QString coverPath = Cover::Utils::coverDirectory(coverToken + "." + extension);

		QList<QUrl> urls;
		for(const QString& url : cdu)
		{
			urls << QUrl(url);
		}

		cl = coverLocation(urls, coverPath);
	}

	else if(md.albumId() >= 0)
	{
		Album album;
		{
			album.setId(md.albumId());
			album.setName(md.album());
			album.setArtists({md.artist()});
			album.setAlbumArtist(md.albumArtist());
			album.setDatabaseId(md.databaseId());
			album.setPathHint({md.filepath()});
		}

		cl = xcoverLocation(album);
	}

	if(!cl.isValid())
	{
		if(md.radioMode() == RadioMode::Station)
		{
			cl = coverLocationRadio(md.radioStation());
		}

		else if(!md.album().isEmpty() && !md.artist().isEmpty())
		{
			cl = coverLocation(md.album(), md.artist());
		}
	}

	bool hasCoverArt;
	if(checkForCoverart) {
		hasCoverArt = Tagging::Covers::hasCover(md.filepath());
	}

	else {
		hasCoverArt = bool(md.customField("has_album_art").toInt());
	}

	if(cl.audioFileSource().isEmpty() && !md.filepath().isEmpty() && hasCoverArt) {
		cl.setAudioFileSource(md.filepath(), cl.coverPath());
	}

	if(cl.searchUrls().isEmpty())
	{
		const QList<Url> urls = extract_downloadUrls(&md);
		cl.setSearchUrls(urls);
		QString identifier = QString("CL:By metadata: %1 by %2 with %3 direct download urls")
			.arg(md.album())
			.arg(md.artist())
			.arg(urls.size());

		cl.setIdentifier(identifier);
	}

	else {
		cl.setIdentifier("CL:By metadata: " + md.album() + " by " + md.artist());
	}

	cl.setLocalPathHints(QStringList{md.filepath()});


	return cl;
}

Location Location::coverLocation(const QList<QUrl>& urls, const QString& targetPath)
{
	QList<Url> fetchUrls;
	QString merged;
	for(const QUrl& url : urls)
	{
		merged += url.toString();
		fetchUrls <<  FetchManager::instance()->directFetcherUrl(url.toString());
	}

	const QString token = QString("Direct_") + Cover::Utils::calcCoverToken(merged, "");

	Location cl;
	{
		cl.setValid(true);
		cl.setCoverPath(targetPath);
		cl.setSearchUrls(fetchUrls);
		cl.setIdentifier("CL:By direct download url: " + merged);
		cl.setHash(token);
	}

	return cl;
}

Location Location::coverLocation(const QUrl& url, const QString& targetPath)
{
	return coverLocation(QList<QUrl>{url}, targetPath);
}

bool Location::isValid() const
{
	return m->valid;
}

QString Location::preferredPath() const
{
	if(!m->valid){
		return Location::invalidPath();
	}

	// first search for cover in track
	if(hasAudioFileSource())
	{
		bool target_exists = File::exists(this->audioFileTarget());
		if(!target_exists)
		{
			const QPixmap pm = Tagging::Covers::extractCover(this->audioFileSource());
			if(!pm.isNull())
			{
				target_exists = pm.save(this->audioFileTarget());
			}
		}

		if(target_exists)
		{
			return audioFileTarget();
		}
	}

	if(!m->localPathHints.isEmpty())
	{
		return localPath();
	}

	return Location::invalidPath();
}

QString Location::alternativePath() const
{
	QString dir, filename;
	Util::File::splitFilename(coverPath(), dir, filename);
	filename.prepend("alt_");

	return dir + QDir::separator() + filename;
}


void Location::setValid(bool b)
{
	m->valid = b;
}

void Location::setIdentifier(const QString& identifier)
{
	m->identifier = identifier;
}

void Location::setCoverPath(const QString& coverPath)
{
	m->coverPath = coverPath;
}

QString Location::coverPath() const
{
	return m->coverPath;
}


QString Location::identifer() const
{
	return m->identifier;
}

UrlList Location::searchUrls() const
{
	if(m->freetextSearch) {
		return m->searchTermUrls;
	}

	else {
		return m->searchUrls;
	}
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
	auto* cfm = Fetcher::Manager::instance();

	m->searchTerm = searchTerm;
	m->searchTermUrls = cfm->searchAddresses(searchTerm);
}

void Location::setSearchTerm(const QString& searchTerm,
							   const QString& cover_fetcher_identifier)
{
	auto* cfm = Fetcher::Manager::instance();

	m->searchTerm = searchTerm;
	m->searchTermUrls = cfm->searchAddresses(searchTerm, cover_fetcher_identifier);
}

void Location::setSearchUrls(const UrlList& urls)
{
	m->searchUrls = urls;
}

void Location::enableFreetextSearch(bool b)
{
	m->freetextSearch = b;
}

bool Location::isFreetextSearchEnabled() const
{
	return m->freetextSearch;
}


bool Location::hasAudioFileSource() const
{
	return
	(
		(m->audioFileTarget.size() > 0) &&
		(m->audioFileSource.size() > 0) &&
		(File::exists(m->audioFileSource))
	);
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

	auto [dir, filename] = File::splitFilename(coverPath);

	m->audioFileSource = audioFilepath;
	m->audioFileTarget = QString("%1/fromtag_%2").arg(dir).arg(filename);

	return true;
}


QString Location::localPath() const
{
	const QString dir = localPathDir();
	if(dir.isEmpty()) {
		return QString();
	}

	const QString linkPath = coverPath();
	if(linkPath.isEmpty()){
		return QString();
	}

	const QFileInfo info(linkPath);
	if(info.exists())
	{
		if(info.isSymLink())
		{
			// delete broken link
			if(!Util::File::exists(info.symLinkTarget())){
				Util::File::deleteFiles({linkPath});
			}

			else { // symlink ok
				return linkPath;
			}
		}

		else if(info.isFile()) {
			return linkPath;
		}

		else {
			spLog(Log::Warning, "CoverLocation") << "Cover path is no symlink and no regular file";
			return QString();
		}
	}

	const QStringList localPaths = Cover::LocalSearcher::coverPathsFromPathHint(dir);
	if(localPaths.isEmpty()) {
		return QString();
	}

	Util::File::createSymlink(localPaths.first(), linkPath);
	return linkPath;
}

QString Location::localPathDir() const
{
	Util::Set<QString> parent_dirs;
	const QStringList lph = localPathHints();

	for(const QString& localPath : lph)
	{
		const QFileInfo fi(localPath);
		if(!fi.exists()){
			continue;
		}

		if(fi.isFile()){
			parent_dirs << Util::File::getParentDirectory(localPath);
		}

		else if(fi.isDir()){
			parent_dirs << localPath;
		}

		if(parent_dirs.size() > 1){
			break;
		}
	}

	if(parent_dirs.isEmpty() || parent_dirs.size() > 1){
		return QString();
	}

	return parent_dirs.first();
}


QStringList Location::localPathHints() const
{
	return m->localPathHints;
}


void Location::setLocalPathHints(const QStringList& path_hints)
{
	m->localPathHints.clear();
	for(const QString& path_hint : path_hints)
	{
		if(!Util::File::isWWW(path_hint))
		{
			m->localPathHints << path_hint;
		}
	}
}

QString Location::hash() const
{
	return m->hash;
}

void Location::setHash(const QString& hash)
{
	m->hash	= hash;
}

QString Location::toString() const
{
	return	"Cover Location: Valid? " + QString::number(m->valid) + ", "
//			"Preferred Path: " + preferredPath() + ", "
//			"Search Urls: " + searchUrls().join(',') + ", "
			"Search Term: " + searchTerm() + ", "
											  "Identifier: " + identifer();
}

