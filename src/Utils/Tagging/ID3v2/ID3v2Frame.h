/* AbstractFrame.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef SAYONARA_ID3V2_FRAME_H
#define SAYONARA_ID3V2_FRAME_H

#include "Utils/Tagging/AbstractFrame.h"

#include <QString>

#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>

#include <optional>

namespace ID3v2
{
	template<typename ModelType_t, typename FrameType_t>
	class ID3v2Frame :
		protected Tagging::AbstractFrame<TagLib::ID3v2::Tag>
	{
		protected:
			FrameType_t* mFrame {nullptr};

		protected:
			virtual TagLib::ID3v2::Frame* createId3v2Frame() = 0;

			virtual void mapDataToFrame(const ModelType_t& model, FrameType_t* frame) = 0;
			virtual std::optional<ModelType_t> mapFrameToData(const FrameType_t* frame) const = 0;

		public:
			ID3v2Frame(TagLib::ID3v2::Tag* tag, const char* four) :
				Tagging::AbstractFrame<TagLib::ID3v2::Tag>(tag, four)
			{
				const auto byteVector = TagLib::ByteVector(four, 4);
				const auto frameListMap = tag->frameListMap();
				const auto frameList = frameListMap[byteVector];
				if(!frameList.isEmpty())
				{
					mFrame = dynamic_cast<FrameType_t*>(frameList.front());
				}
			}

			virtual ~ID3v2Frame() = default;

			virtual bool read(ModelType_t& data) const
			{
				if(mFrame)
				{
					if(const auto optData = mapFrameToData(mFrame); optData.has_value())
					{
						data = optData.value();
						return true;
					}
				}

				return false;
			}

			virtual bool write(const ModelType_t& data)
			{
				if(!tag())
				{
					return false;
				}

				auto created = false;
				if(!mFrame)
				{
					mFrame = dynamic_cast<FrameType_t*>(createId3v2Frame());
					if(!mFrame)
					{
						return false;
					}

					created = true;
				}

				mapDataToFrame(data, mFrame);

				if(created)
				{
					tag()->addFrame(mFrame);
				}

				return true;
			}

			bool isFrameAvailable() const
			{
				return (mFrame != nullptr);
			}

			FrameType_t* frame()
			{
				return mFrame;
			}
	};
}

#endif // SAYONARA_ID3V2_FRAME_H
