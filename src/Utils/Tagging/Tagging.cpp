/* id3.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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

#include "Tagging.h"
#include "Tagging/TaggingCover.h"
#include "ID3v2/Popularimeter.h"
#include "ID3v2/Discnumber.h"
#include "ID3v2/AlbumArtist.h"
#include "Xiph/AlbumArtist.h"
#include "Xiph/PopularimeterFrame.h"
#include "Xiph/DiscnumberFrame.h"
#include "MP4/AlbumArtist.h"
#include "MP4/DiscnumberFrame.h"
#include "MP4/PopularimeterFrame.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Logger/Logger.h"

#include <taglib/tag.h>
#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/oggfile.h>
#include <taglib/oggflacfile.h>
#include <taglib/flacfile.h>
#include <taglib/tbytevector.h>
#include <taglib/tbytevectorstream.h>
#include <taglib/id3v1tag.h>
#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>

#include <QFileInfo>
#include <QRegExp>
#include <QStringList>

using namespace Tagging::Utils;


bool Tagging::Utils::is_valid_file(const TagLib::FileRef& f)
{
	if( f.isNull() ||
		!f.tag() ||
		!f.file() ||
		!f.file()->isValid() )
	{
		return false;
	}

	return true;
}

bool Tagging::Utils::getMetaDataOfFile(MetaData& md, Quality quality)
{
	bool success;

	QFileInfo fi(md.filepath());
	md.filesize = fi.size();
	if(fi.size() <= 0){
		return false;
	}

	TagLib::AudioProperties::ReadStyle read_style = TagLib::AudioProperties::Fast;
	bool read_audio_props=true;

	switch(quality)
	{
		case Quality::Quality:
			read_style = TagLib::AudioProperties::Accurate;
			break;
		case Quality::Standard:
			read_style = TagLib::AudioProperties::Average;
			break;
		case Quality::Fast:
			read_style = TagLib::AudioProperties::Fast;
			break;
		case Quality::Dirty:
			read_style = TagLib::AudioProperties::Fast;
			read_audio_props = false;
			break;
	};

	TagLib::FileRef f(
			TagLib::FileName(md.filepath().toUtf8()),
			read_audio_props,
			read_style
	);

	if(!is_valid_file(f)){
		sp_log(Log::Warning, "Tagging") << "Cannot open tags for " << md.filepath() << ": Err 1";
		return false;
	}

	ParsedTag parsed_tag = tag_type_from_fileref(f);
	TagLib::Tag* tag = parsed_tag.tag;

	QString artist = QString::fromUtf8(tag->artist().toCString(true));
	QString album = QString::fromUtf8(tag->album().toCString(true));
	QString title = QString::fromUtf8(tag->title().toCString(true));
	QString genre = QString::fromUtf8(tag->genre().toCString(true));
	QString comment = QString::fromUtf8(tag->comment().toCString(true));

	QString album_artist;

	Models::Discnumber discnumber;
	Models::Popularimeter popularimeter;
	if(parsed_tag.type == TagType::ID3v2)
	{
		auto id3v2 = parsed_tag.id3_tag();
		ID3v2::AlbumArtistFrame album_artist_frame(id3v2);
		success = album_artist_frame.read(album_artist);
		if(success){
			md.set_album_artist(album_artist);
		}

		ID3v2::PopularimeterFrame popularimeter_frame(id3v2);
		success = popularimeter_frame.read(popularimeter);
		if(success){
			md.rating = popularimeter.get_rating();
		}

		ID3v2::DiscnumberFrame discnumber_frame(id3v2);
		success = discnumber_frame.read(discnumber);
		if(success){
			md.discnumber = discnumber.disc;
			md.n_discs = discnumber.n_discs;
		}
	}

	else if(parsed_tag.type == TagType::Xiph)
	{
		auto xiph = parsed_tag.xiph_tag();

		Xiph::AlbumArtistFrame album_artist_frame(xiph);
		success = album_artist_frame.read(album_artist);
		if(success){
			md.set_album_artist(album_artist);
		}

		Xiph::PopularimeterFrame popularimeter_frame(xiph);
		success = popularimeter_frame.read(popularimeter);
		if(success){
			md.rating = popularimeter.get_rating();
		}

		Xiph::DiscnumberFrame discnumber_frame(xiph);
		success = discnumber_frame.read(discnumber);
		if(success){
			md.discnumber = discnumber.disc;
			md.n_discs = discnumber.n_discs;
		}
	}

	else if(parsed_tag.type == TagType::MP4)
	{
		auto mp4 = parsed_tag.mp4_tag();

		MP4::AlbumArtistFrame album_artist_frame(mp4);
		success = album_artist_frame.read(album_artist);
		if(success){
			md.set_album_artist(album_artist);
		}

		MP4::DiscnumberFrame discnumber_frame(mp4);
		success = discnumber_frame.read(discnumber);
		if(success){
			md.discnumber = discnumber.disc;
			md.n_discs = discnumber.n_discs;
		}

		MP4::PopularimeterFrame popularimeter_frame(mp4);

		success = popularimeter_frame.read(popularimeter);
		if(success){
			md.rating = popularimeter.get_rating();
		}

		//sp_log(Log::Debug, this) << "Read rating " << (int) md.rating << ": " << success;
	}

	uint year = tag->year();
	uint track = tag->track();

	int bitrate=0;
	int length=0;

	if( quality != Quality::Dirty ){
		bitrate = f.audioProperties()->bitrate() * 1000;
		length = f.audioProperties()->length() * 1000;
	}

	QStringList genres;
	QString genre_str = ::Util::cvt_str_to_first_upper(genre);
	genres = genre_str.split(QRegExp(",|/|;"));
	for(int i=0; i<genres.size(); i++) {
		genres[i] = genres[i].trimmed();
	}

	genres.removeDuplicates();
	genres.removeAll("");

	md.set_album(album);
	md.set_artist(artist);
	md.set_title(title);
	md.length_ms = length;
	md.year = year;
	md.track_num = track;
	md.bitrate = bitrate;
	md.set_genres(genres);
	md.discnumber = discnumber.disc;
	md.n_discs = discnumber.n_discs;
	md.rating = popularimeter.get_rating();
	md.set_comment(comment);
	md.add_custom_field("has_album_art", "", QString::number(Tagging::Covers::has_cover(parsed_tag)));

	if(md.title().length() == 0)
	{
		QString dir, filename;
		::Util::File::split_filename(md.filepath(), dir, filename);

		if(filename.size() > 4){
			filename = filename.left(filename.length() - 4);
		}

		md.set_title(filename);
	}

	return true;
}


bool Tagging::Utils::setMetaDataOfFile(const MetaData& md)
{
	QString filepath = md.filepath();
	QFileInfo info(filepath);
	if(info.size() <= 0){
		return false;
	}

	TagLib::FileRef f(TagLib::FileName(filepath.toUtf8()));

	if(!is_valid_file(f)){
		sp_log(Log::Warning, "Tagging") << "Cannot open tags for " << md.filepath() << ": Err 2";
		return false;
	}

	bool success;

	TagLib::String album(md.album().toUtf8().data(), TagLib::String::UTF8);
	TagLib::String artist(md.artist().toUtf8().data(), TagLib::String::UTF8);
	TagLib::String title(md.title().toUtf8().data(), TagLib::String::UTF8);
	TagLib::String genre(md.genres_to_string().toUtf8().data(), TagLib::String::UTF8);
	TagLib::String comment(md.comment().toUtf8().data(), TagLib::String::UTF8);

	ParsedTag parsed_tag = tag_type_from_fileref(f);
	TagLib::Tag* tag = parsed_tag.tag;

	tag->setAlbum(album);
	tag->setArtist(artist);
	tag->setTitle(title);
	tag->setGenre(genre);
	tag->setYear(md.year);
	tag->setTrack(md.track_num);
	tag->setComment(comment);

	Models::Popularimeter popularimeter("sayonara player", 0, 0);
	popularimeter.set_rating(md.rating);
	Models::Discnumber discnumber(md.discnumber, md.n_discs);

	if(parsed_tag.type == TagType::ID3v2)
	{
		auto id3v2 = parsed_tag.id3_tag();
		ID3v2::PopularimeterFrame popularimeter_frame(id3v2);
		popularimeter_frame.write(popularimeter);

		ID3v2::DiscnumberFrame discnumber_frame(id3v2);
		discnumber_frame.write(discnumber);

		ID3v2::AlbumArtistFrame album_artist_frame(id3v2);
		album_artist_frame.write(md.album_artist());
	}

	else if(parsed_tag.type == TagType::Xiph)
	{
		auto xiph = parsed_tag.xiph_tag();
		Xiph::PopularimeterFrame popularimeter_frame(xiph);
		popularimeter_frame.write(popularimeter);

		Xiph::DiscnumberFrame discnumber_frame(xiph);
		discnumber_frame.write(discnumber);

		Xiph::AlbumArtistFrame album_artist_frame(xiph);
		album_artist_frame.write(md.album_artist());
	}

	else if(parsed_tag.type == TagType::MP4)
	{
		auto mp4 = parsed_tag.mp4_tag();
		MP4::AlbumArtistFrame album_artist_frame(mp4);
		album_artist_frame.write(md.album_artist());

		MP4::DiscnumberFrame discnumber_frame(mp4);
		discnumber_frame.write(discnumber);

		MP4::PopularimeterFrame popularimeter_frame(mp4);
		popularimeter_frame.write(popularimeter);
	}

	success = f.save();
	if(!success){
		sp_log(Log::Warning, "Tagging") << "Could not save " << md.filepath();
	}

	return true;
}

Tagging::ParsedTag Tagging::Utils::tag_type_from_fileref(const TagLib::FileRef& f)
{
	ParsedTag ret;

	ret.tag = f.tag();
	ret.type = TagType::Unsupported;

	TagLib::MPEG::File* mpg = dynamic_cast<TagLib::MPEG::File*>(f.file());
	if(mpg)
	{
		if(mpg->hasID3v2Tag())
		{
			ret.tag = mpg->ID3v2Tag();
			ret.type = TagType::ID3v2;
			return ret;
		}

		else if(mpg->hasID3v1Tag())
		{
			ret.tag = mpg->ID3v1Tag();
			ret.type = TagType::ID3v1;
			return ret;
		}
	}

	TagLib::FLAC::File* flac = dynamic_cast<TagLib::FLAC::File*>(f.file());
	if(flac)
	{
		if(flac->hasXiphComment())
		{
			ret.tag = flac->xiphComment();
			ret.type = TagType::Xiph;
			return ret;
		}

		else if(flac->hasID3v2Tag())
		{
			ret.tag = flac->ID3v2Tag();
			ret.type = TagType::ID3v2;
			return ret;
		}

		else if(flac->hasID3v1Tag())
		{
			ret.tag = flac->ID3v1Tag();
			ret.type = TagType::ID3v1;
		}
	}

	TagLib::Tag* tag = f.tag();
	if(dynamic_cast<TagLib::ID3v2::Tag*>(tag) != nullptr)
	{
		ret.type = TagType::ID3v2;
	}

	else if(dynamic_cast<TagLib::ID3v1::Tag*>(tag) != nullptr)
	{
		ret.type = TagType::ID3v1;
	}

	else if(dynamic_cast<TagLib::Ogg::XiphComment*>(tag) != nullptr)
	{
		ret.type = TagType::Xiph;
	}

	else if(dynamic_cast<TagLib::MP4::Tag*>(tag) != nullptr)
	{
		ret.type = TagType::MP4;
	}

	return ret;
}


Tagging::TagType Tagging::Utils::get_tag_type(const QString &filepath)
{
	TagLib::FileRef f(TagLib::FileName(filepath.toUtf8()));
	if(!is_valid_file(f)){
		return TagType::Unknown;
	}

	ParsedTag parsed = tag_type_from_fileref(f);
	return parsed.type;
}

QString Tagging::Utils::tag_type_to_string(TagType type)
{
	switch(type){
		case TagType::ID3v1:
			return "ID3v1";
		case TagType::ID3v2:
			return "ID3v2";
		case TagType::Xiph:
			return "Xiph";
		case TagType::MP4:
			return "MP4";
		case TagType::Unknown:
			return "Unknown";
		default:
			return "Partially unsupported";
	}
}



