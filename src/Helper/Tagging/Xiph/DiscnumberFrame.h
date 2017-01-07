#ifndef DISCNUMBERXIPHFRAME_H
#define DISCNUMBERXIPHFRAME_H

#include "Tagging/Models/Discnumber.h"
#include "AbstractFrame.h"

namespace Xiph
{
	class DiscnumberFrame :
		public Xiph::AbstractFrame<Models::Discnumber>
	{
	public:
		DiscnumberFrame(TagLib::Tag* tag);
		virtual ~DiscnumberFrame();

	protected:
		bool map_tag_to_model(const TagLib::String& value, Models::Discnumber& model);
		bool map_model_to_tag(const Models::Discnumber& model, TagLib::Ogg::XiphComment* tag);
	};
}

#endif // DISCNUMBERFRAME_H