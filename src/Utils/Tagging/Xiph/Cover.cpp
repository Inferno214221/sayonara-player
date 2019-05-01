/* Cover.cpp */

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



#include "Cover.h"
#include <taglib/flacpicture.h>
#include <taglib/flacfile.h>
#include <taglib/oggfile.h>

namespace TL=TagLib;

Xiph::CoverFrame::CoverFrame(TagLib::Ogg::XiphComment* tag) :
	Xiph::XiphFrame<Models::Cover>(tag, "")
{}

Xiph::CoverFrame::~CoverFrame() {}

bool Xiph::CoverFrame::is_frame_found() const
{
	return this->tag()->contains("METADTA_BLOCK_PICTURE");
}

bool Xiph::CoverFrame::map_tag_to_model(Models::Cover& model)
{

#if TAGLIB_MINOR_VERSION < 10
	Q_UNUSED(model)
	return false;
#else

	TL::Ogg::XiphComment* xiph = this->tag();
	TL::List<TL::FLAC::Picture*> pictures = xiph->pictureList();
	if(pictures.isEmpty())
	{
		model.mime_type = QString();
		model.image_data.clear();
		return true;
	}

	TL::FLAC::Picture* pic_of_interest = nullptr;
	for(TL::FLAC::Picture* pic : pictures)
	{
		if(pic->type() == TL::FLAC::Picture::FrontCover)
		{
			if(pic->data().size() < 100){
				continue;
			}

			pic_of_interest = pic;
		}

		else if(!pic_of_interest && pic->type() == TL::FLAC::Picture::Other)
		{
			if(pic->data().size() < 100){
				continue;
			}

			pic_of_interest = pic;
		}
	}

	if(pic_of_interest == nullptr)
	{
		pic_of_interest = pictures[0];
	}

	{
		TL::ByteVector data = pic_of_interest->data();
		model.image_data = QByteArray(data.data(), static_cast<int>(data.size()));
		model.mime_type = cvt_string(pic_of_interest->mimeType());
	}

	return true;
#endif
}

bool Xiph::CoverFrame::map_model_to_tag(const Models::Cover& model)
{

#if TAGLIB_MINOR_VERSION < 10
	Q_UNUSED(model)
	return false;
#else

	this->tag()->removeAllPictures();

	unsigned int length = static_cast<unsigned int>(model.image_data.size());

	TL::ByteVector img_data(model.image_data.data(), length);

	TL::Ogg::XiphComment* tag = this->tag();
	TL::FLAC::Picture* pic = new TL::FLAC::Picture();
	pic->setType(TL::FLAC::Picture::FrontCover);
	pic->setMimeType(cvt_string(model.mime_type));
	pic->setDescription(TL::String("Front Cover By Sayonara"));
	pic->setData(TL::ByteVector(model.image_data.data(), length) );
	tag->addPicture(pic); // do not delete the picture, because tag will take ownership

	return true;
#endif
}
