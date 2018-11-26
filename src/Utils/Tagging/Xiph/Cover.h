#ifndef XIPH_COVER_H
#define XIPH_COVER_H

#include "Utils/Tagging/Xiph/XiphFrame.h"
#include "Utils/Tagging/Models/Cover.h"
#include "XiphFrame.h"

/**
 * @ingroup Tagging
 */
namespace Xiph
{
	class CoverFrame :
		public XiphFrame<Models::Cover>
	{
		public:
			CoverFrame(TagLib::Ogg::XiphComment* tag);
			~CoverFrame() override;

			bool is_frame_found() const override;

		protected:
			bool map_tag_to_model(Models::Cover& model) override;
			bool map_model_to_tag(const Models::Cover& model) override;
	};
}

#endif // COVER_H
