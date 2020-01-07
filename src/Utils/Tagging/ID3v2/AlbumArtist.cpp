/* AlbumArtist.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "AlbumArtist.h"
#include "taglib/textidentificationframe.h"
#include "taglib/tstring.h"

ID3v2::AlbumArtistFrame::AlbumArtistFrame(TagLib::ID3v2::Tag* tag) :
	ID3v2Frame<QString, TagLib::ID3v2::TextIdentificationFrame>(tag, "TPE2") {}

ID3v2::AlbumArtistFrame::~AlbumArtistFrame() {}

void ID3v2::AlbumArtistFrame::map_model_to_frame(const QString& model, TagLib::ID3v2::TextIdentificationFrame* frame)
{
	QByteArray data = model.toUtf8();
	TagLib::String str(data.constData(), TagLib::String::UTF8);
	frame->setText(str);
}

void ID3v2::AlbumArtistFrame::map_frame_to_model(const TagLib::ID3v2::TextIdentificationFrame* frame, QString& model)
{
	TagLib::String tag_str = frame->toString();
	QString str = QString::fromUtf8( tag_str.toCString(true) );
	model = str;
}

TagLib::ID3v2::Frame* ID3v2::AlbumArtistFrame::create_id3v2_frame()
{
	return new TagLib::ID3v2::TextIdentificationFrame("TPE2", TagLib::String::UTF8);
}
