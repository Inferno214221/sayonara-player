/* StreamRecorder.h */

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

#ifndef STREAMRECORDER_H
#define STREAMRECORDER_H

#include "Utils/Pimpl.h"

#include <QObject>

class MetaData;

namespace PipelineExtensions
{
	class StreamRecordable;
}

namespace Tagging
{
	class TagWriter;
}

namespace Util
{
	class FileSystem;
}

namespace StreamRecorder
{
	class StreamRecorder :
		public QObject
	{
		PIMPL(StreamRecorder)

		public:
			StreamRecorder(std::shared_ptr<Util::FileSystem> fileSystem,
			               std::shared_ptr<Tagging::TagWriter> tagWriter,
			               std::shared_ptr<PipelineExtensions::StreamRecordable> streamRecordable,
			               QObject* parent = nullptr);
			~StreamRecorder() override;

			void startNewSession(const MetaData& track);
			void updateMetadata(const MetaData& track);
			void endSession();
			[[nodiscard]] bool isRecording() const;

		private: // NOLINT(readability-redundant-access-specifiers)
			bool save();
			void setCurrentTrack(const MetaData& track);
	};
}

#endif // STREAMRECORDER_H
