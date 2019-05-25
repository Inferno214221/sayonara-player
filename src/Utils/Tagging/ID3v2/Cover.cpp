/* Cover.cpp */
/* Sayonara Player

  Copyright (C) 2011-2019  Lucio Carreras
 
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Cover.h"

ID3v2::CoverFrame::CoverFrame(TagLib::ID3v2::Tag* tag) :
	ID3v2Frame<Models::Cover, TagLib::ID3v2::AttachedPictureFrame>(tag, "APIC") {}

ID3v2::CoverFrame::~CoverFrame() {}

void  ID3v2::CoverFrame::map_model_to_frame(const Models::Cover& model, TagLib::ID3v2::AttachedPictureFrame* frame)
{
	TagLib::String description = TagLib::String("Cover by Sayonara Player");
	TagLib::String::Type encoding = TagLib::String::Latin1;
	TagLib::String mime_type = TagLib::String(model.mime_type.toLatin1().constData());
	TagLib::ID3v2::AttachedPictureFrame::Type type = TagLib::ID3v2::AttachedPictureFrame::FrontCover;

	TagLib::ByteVector taglib_image_data;
	const QByteArray& image_data = model.image_data;

	taglib_image_data.setData(image_data.data(), image_data.size());

	TagLib::ByteVector vec, vec_header;
	vec_header = TagLib::ByteVector("APIC", 4);

	frame->setDescription(description);
	frame->setTextEncoding(encoding);
	frame->setMimeType(mime_type);
	frame->setType(type);
	frame->setPicture(taglib_image_data);

	vec = frame->render();

	if( !vec.startsWith(vec_header) ){
		vec = vec_header + vec;
	}

	frame->setData(vec);
}

void ID3v2::CoverFrame::map_frame_to_model(const TagLib::ID3v2::AttachedPictureFrame* frame, Models::Cover& model)
{
	TagLib::ByteVector taglib_image_data = frame->picture();
	TagLib::String mime_type = frame->mimeType();

	model.image_data = QByteArray(taglib_image_data.data(), taglib_image_data.size());
	model.mime_type = QString::fromLatin1(mime_type.toCString(), mime_type.length());
}

TagLib::ID3v2::Frame* ID3v2::CoverFrame::create_id3v2_frame()
{
	return new TagLib::ID3v2::AttachedPictureFrame(TagLib::ByteVector());
}



