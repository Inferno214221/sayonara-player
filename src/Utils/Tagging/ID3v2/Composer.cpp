/* AlbumArtist.cpp */

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

#include "Composer.h"
#include <taglib/textidentificationframe.h>
#include <taglib/tstring.h>

ID3v2::ComposerFrame::ComposerFrame(TagLib::ID3v2::Tag* tag) :
	ID3v2Frame<QString, TagLib::ID3v2::TextIdentificationFrame>(tag, "\xA9wrt") {}

ID3v2::ComposerFrame::~ComposerFrame() {}

void ID3v2::ComposerFrame::map_model_to_frame(const QString& model, TagLib::ID3v2::TextIdentificationFrame* frame)
{
	QByteArray data = model.toUtf8();
	TagLib::String str(data.constData(), TagLib::String::UTF8);
	frame->setText(str);
}

void ID3v2::ComposerFrame::map_frame_to_model(const TagLib::ID3v2::TextIdentificationFrame* frame, QString& model)
{
	TagLib::String tag_str = frame->toString();
	QString str = QString::fromUtf8( tag_str.toCString(true) );
	model = str;
}

TagLib::ID3v2::Frame* ID3v2::ComposerFrame::create_id3v2_frame()
{
	return new TagLib::ID3v2::TextIdentificationFrame("\xA9wrt", TagLib::String::UTF8);
}
