/* CoverLocation.cpp */

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

static QList<Url> extract_download_urls(const LibraryItem* item)
{
	QList<Url> urls;
	const QStringList cdu = item->cover_download_urls();
	for(const QString& url : cdu)
	{
		urls << Url(true, "Direct", url);
	}

	return urls;
}

struct Location::Private
{
	QString			search_term;		// Term provided to search engine
	UrlList	search_urls;		// Search url where to fetch covers
	UrlList	search_term_urls;	// Search urls where to fetch cover when using freetext search
	QStringList		local_path_hints;
	StringMap		all_search_urls;	// key = identifier of coverfetcher, value = search url
	QString			cover_path;			// cover_path path, in .Sayonara, where cover is stored. Ignored if local_paths are not empty
	QString			identifier;			// Some human readable identifier with methods where invokded
	QString			audio_file_source;	// A saved cover from an audio file
	QString			audio_file_target;

	QString			hash;				// A unique identifier, mostly referred to as the cover token

	bool			freetext_search;
	bool			valid;				// valid if CoverLocation object contains a valid download url

	Private() :
		freetext_search(false),
		valid(false)
	{}

	Private(const Private& other) :
		CASSIGN(search_term),
		CASSIGN(search_urls),
		CASSIGN(search_term_urls),
		CASSIGN(local_path_hints),
		CASSIGN(all_search_urls),
		CASSIGN(cover_path),
		CASSIGN(identifier),
		CASSIGN(audio_file_source),
		CASSIGN(audio_file_target),
		CASSIGN(hash),
		CASSIGN(freetext_search),
		CASSIGN(valid)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(search_term);
		ASSIGN(search_urls);
		ASSIGN(search_term_urls);
		ASSIGN(local_path_hints);
		ASSIGN(all_search_urls);
		ASSIGN(cover_path);
		ASSIGN(identifier);
		ASSIGN(audio_file_source);
		ASSIGN(audio_file_target);
		ASSIGN(hash);
		ASSIGN(freetext_search);
		ASSIGN(valid);

		return (*this);
	}
};


void Location::set_valid(bool b)
{
	m->valid = b;
}

void Location::set_identifier(const QString& identifier)
{
	m->identifier = identifier;
}

void Location::set_cover_path(const QString& cover_path)
{
	m->cover_path = cover_path;
}

QString Location::cover_path() const
{
	return m->cover_path;
}

Location::Location()
{
	qRegisterMetaType<Location>("CoverLocation");

	m = Pimpl::make<Location::Private>();
}

Location::~Location() {}

Location::Location(const Location& other)
{
	m = Pimpl::make<Location::Private>(*(other.m));
}

Location& Location::operator=(const Location& other)
{
	*m = *(other.m);
	return *this;
}


QString Location::invalid_path()
{
	return invalid_location().cover_path();
}


Location Location::invalid_location()
{
	Location cl;

	cl.set_valid(false);
	cl.set_cover_path(::Util::share_path("logo.png"));
	cl.set_search_urls(UrlList());
	cl.set_search_term(QString());
	cl.set_identifier("Invalid location");
	cl.set_audio_file_source(QString(), QString());
	cl.set_local_path_hints(QStringList());

	return cl;
}


Location Location::cover_location(const QString& album_name, const QString& artist_name)
{
	using namespace Cover::Fetcher;
	if(album_name.trimmed().isEmpty() && artist_name.trimmed().isEmpty())
	{
		return invalid_location();
	}

	QString cover_token = Cover::Utils::calc_cover_token(artist_name, album_name);
	QString cover_path = Cover::Utils::cover_directory( cover_token + ".jpg" );
	Fetcher::Manager* cfm = Fetcher::Manager::instance();

	Location ret;

	ret.set_valid(true);
	ret.set_cover_path(cover_path);
	ret.set_hash(cover_token);
	ret.set_search_term(artist_name + " " + album_name);
	ret.set_search_urls(cfm->album_addresses(artist_name, album_name, true));
	ret.set_identifier("CL:By album: " + album_name + " by " + artist_name);

	return ret;
}

Location Location::cover_location(const QString& album_name, const QStringList& artists)
{
	QString major_artist = ArtistList::get_major_artist(artists);
	return cover_location(album_name, major_artist);
}


Location Location::xcover_location(const Album& album)
{
	Location cl;

	{ //setup basic CoverLocation
		if( album.album_artists().size() == 1)
		{
			cl = Location::cover_location(album.name(), album.album_artists().at(0));
		}

		else if(album.artists().size() > 1)
		{
			cl = Location::cover_location(album.name(), album.artists());
		}

		else if(album.artists().size() == 1)
		{
			cl = Location::cover_location(album.name(), album.artists().at(0));
		}

		else
		{
			cl = Location::cover_location(album.name(), "");
		}

		QList<Url> urls = extract_download_urls( &album );
		if(!urls.isEmpty()) {
			cl.set_search_urls(urls);
		}
	}

	// setup local paths. No audio file source. That may last too long
	{
		QStringList path_hints = album.path_hint();
		if(!path_hints.isEmpty())
		{
			cl.set_local_path_hints(path_hints);
			cl.set_audio_file_source(path_hints.first(), cl.cover_path());
		}
	}

	cl.set_search_term(album.name() + " " + ArtistList::get_major_artist(album.artists()));

	return cl;
}

Location Location::cover_location(const Artist& artist)
{
	Location cl = Location::cover_location(artist.name());

	QList<Url> urls = extract_download_urls(&artist);
	if(!urls.isEmpty()) {
		cl.set_search_urls(urls);
	}

	cl.set_search_term(artist.name());
	cl.set_identifier("CL:By artist: " + artist.name());

	return cl;
}


Location Location::cover_location(const QString& artist)
{
	if(artist.trimmed().isEmpty()) {
		return invalid_location();
	}

	QString cover_token = QString("artist_") + Cover::Utils::calc_cover_token(artist, "");
	QString cover_path = Cover::Utils::cover_directory(cover_token + ".jpg");

	auto* cfm = Fetcher::Manager::instance();

	Location ret;

	ret.set_valid(true);
	ret.set_cover_path(cover_path);
	ret.set_search_urls(cfm->artist_addresses(artist, true));
	ret.set_search_term(artist);
	ret.set_identifier("CL:By artist name: " + artist);
	ret.set_hash(cover_token);

	return ret;
}

Location Location::cover_location(const MetaData& md)
{
	return Location::cover_location(md, true);
}

Location Location::cover_location(const MetaData& md, bool check_for_coverart)
{
	Location cl;

	const QStringList cdu = md.cover_download_urls();
	if(!cdu.isEmpty())
	{
		QString extension = File::get_file_extension(cdu.first());

		QString cover_token = Cover::Utils::calc_cover_token(md.artist(), md.album());
		QString cover_path = Cover::Utils::cover_directory(cover_token + "." + extension);

		QList<QUrl> urls;
		for(const QString& url : cdu)
		{
			urls << QUrl(url);
		}

		cl = cover_location(urls, cover_path);
	}

	else if(md.album_id >= 0)
	{
		Album album;
		album.id = md.album_id;
		album.set_name(md.album());
		album.set_artists({md.artist()});
		album.set_album_artists({md.album_artist()});
		album.set_db_id(md.db_id());
		album.set_path_hint({md.filepath()});

		cl = xcover_location(album);
	}

	if(!cl.is_valid() && !md.album().isEmpty() && !md.artist().isEmpty()){
		cl = cover_location(md.album(), md.artist());
	}

	bool has_cover_art;
	if(check_for_coverart) {
		has_cover_art = Tagging::Covers::has_cover(md.filepath());
	}

	else {
		has_cover_art = bool(md.get_custom_field("has_album_art").toInt());
	}

	if(cl.audio_file_source().isEmpty() && !md.filepath().isEmpty() && has_cover_art) {
		cl.set_audio_file_source(md.filepath(), cl.cover_path());
	}

	if(cl.search_urls(true).isEmpty())
	{
		QList<Url> urls = extract_download_urls(&md);
		cl.set_search_urls(urls);
	}

	cl.set_local_path_hints(QStringList{md.filepath()});
	cl.set_identifier("CL:By metadata: " + md.album() + " by " + md.artist());

	return cl;
}

Location Location::cover_location(const QList<QUrl>& urls, const QString& target_path)
{
	QList<Url> fetch_urls;
	QString merged;
	for(QUrl url : urls)
	{
		merged += url.toString();
		fetch_urls << Url(true, "Direct", url.toString());
	}

	QString token = QString("Direct_") + Cover::Utils::calc_cover_token(merged, "");

	Location cl;
	cl.set_valid(true);
	cl.set_cover_path(target_path);
	cl.set_search_urls(fetch_urls);
	cl.set_identifier("CL:By direct download url: " + merged);
	cl.set_hash(token);

	return cl;
}

Location Location::cover_location(const QUrl& url, const QString& target_path)
{
	return cover_location(QList<QUrl>{url}, target_path);
}

bool Location::is_valid() const
{
	return m->valid;
}

QString Location::preferred_path() const
{
	if(!m->valid){
		return invalid_path();
	}

	// first search for cover in track
	if(has_audio_file_source())
	{
		bool target_exists = File::exists(this->audio_file_target());
		if(!target_exists)
		{
			QPixmap pm = Tagging::Covers::extract_cover(this->audio_file_source());
			if(!pm.isNull())
			{
				target_exists = pm.save(this->audio_file_target());
			}
		}

		if(target_exists)
		{
			return audio_file_target();
		}
	}

	if(!m->local_path_hints.isEmpty())
	{
		return local_path();
	}

	return invalid_path();
}

QString Location::alternative_path() const
{
	QString dir, filename;
	Util::File::split_filename(cover_path(), dir, filename);
	filename.prepend("alt_");

	return dir + QDir::separator() + filename;
}


QString Location::identifer() const
{
	return m->identifier;
}

UrlList Location::search_urls(bool also_inactive) const
{
	UrlList container;
	if(m->freetext_search) {
		container = m->search_term_urls;
	}

	else {
		container = m->search_urls;
	}

	UrlList ret;
	for(Url url : container)
	{
		auto* manager = Manager::instance();
		bool active = manager->is_active(url.identifier());
		url.set_active(active);

		if(active || also_inactive)
		{
			ret << url;
		}
	}

	return ret;
}

Url Location::search_url(int idx) const
{
	if(!Util::between(idx, m->search_urls)){
		return Url();
	}

	return m->search_urls.at(idx);
}

bool Location::has_search_urls() const
{
	return !(m->search_urls.isEmpty());
}


QString Location::search_term() const
{
	return m->search_term;
}

void Location::set_search_term(const QString& search_term)
{
	auto* cfm = Fetcher::Manager::instance();

	m->search_term = search_term;
	m->search_term_urls = cfm->search_addresses(search_term, true);
}

void Location::set_search_term(const QString& search_term,
							   const QString& cover_fetcher_identifier)
{
	auto* cfm = Fetcher::Manager::instance();

	m->search_term = search_term;
	m->search_term_urls = cfm->search_addresses(search_term, cover_fetcher_identifier, true);
}

void Location::set_search_urls(const UrlList& urls)
{
	m->search_urls = urls;
}

void Location::enable_freetext_search(bool b)
{
	m->freetext_search = b;
}

bool Location::is_freetext_search_enabled() const
{
	return m->freetext_search;
}


bool Location::has_audio_file_source() const
{
	return
	(
		(m->audio_file_target.size() > 0) &&
		(m->audio_file_source.size() > 0) &&
		(File::exists(m->audio_file_source))
	);
}

QString Location::audio_file_source() const
{
	return m->audio_file_source;
}

QString Location::audio_file_target() const
{
	return m->audio_file_target;
}

bool Location::set_audio_file_source(const QString& audio_filepath, const QString& cover_path)
{
	m->audio_file_source = QString();
	m->audio_file_target = QString();

	if(audio_filepath.isEmpty() || cover_path.isEmpty())
	{
		return false;
	}

	QString dir, filename;
	File::split_filename(cover_path, dir, filename);
	filename.prepend("fromtag_");

	m->audio_file_source = audio_filepath;
	m->audio_file_target = dir + "/" + filename;

	return true;
}


QString Location::local_path() const
{
	const QString dir = local_path_dir();
	if(dir.isEmpty()) {
		return QString();
	}

	const QString link_path = cover_path();
	if(link_path.isEmpty()){
		return QString();
	}

	const QFileInfo info(link_path);
	if(info.exists())
	{
		if(info.isSymLink())
		{
			// delete broken link
			if(!Util::File::exists(info.symLinkTarget())){
				Util::File::delete_files({link_path});
			}

			else { // symlink ok
				return link_path;
			}
		}

		else if(info.isFile()) {
			return link_path;
		}

		else {
			sp_log(Log::Warning, "CoverLocation") << "Cover path is no symlink and no regular file";
			return QString();
		}
	}

	const QStringList local_paths = Cover::LocalSearcher::cover_paths_from_path_hint(dir);
	if(local_paths.isEmpty()) {
		return QString();
	}

	Util::File::create_symlink(local_paths.first(), link_path);
	return link_path;
}

QString Location::local_path_dir() const
{
	Util::Set<QString> parent_dirs;
	const QStringList lph = local_path_hints();
	for(const QString& local_path : lph)
	{
		QFileInfo fi(local_path);
		if(!fi.exists()){
			continue;
		}

		if(fi.isFile()){
			parent_dirs << Util::File::get_parent_directory(local_path);
		}

		else if(fi.isDir()){
			parent_dirs << local_path;
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


QStringList Location::local_path_hints() const
{
	return m->local_path_hints;
}

void Location::set_local_path_hints(const QStringList& path_hints)
{
	m->local_path_hints.clear();
	for(const QString& path_hint : path_hints)
	{
		if(!Util::File::is_www(path_hint))
		{
			m->local_path_hints << path_hint;
		}
	}
}

QString Location::hash() const
{
	return m->hash;
}

void Location::set_hash(const QString& hash)
{
	m->hash	= hash;
}


QString Location::to_string() const
{
	return	"Cover Location: Valid? " + QString::number(m->valid) + ", "
//			"Preferred Path: " + preferred_path() + ", "
//			"Search Urls: " + search_urls().join(',') + ", "
			"Search Term: " + search_term() + ", "
											  "Identifier: " + identifer();
}

