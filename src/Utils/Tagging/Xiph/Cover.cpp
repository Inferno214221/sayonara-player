#include "Cover.h"
#include <taglib/flacpicture.h>

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
	TL::List<TL::FLAC::Picture*> pictures = this->tag()->pictureList();
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
}

bool Xiph::CoverFrame::map_model_to_tag(const Models::Cover& model)
{
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
}
