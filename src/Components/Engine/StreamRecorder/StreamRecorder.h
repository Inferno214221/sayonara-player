/* StreamRecorder.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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
class PlayManager;

namespace PipelineExtensions
{
	class StreamRecordable;
}

namespace StreamRecorder
{
	class StreamRecorder :
		public QObject
	{
		PIMPL(StreamRecorder)

		public:
			StreamRecorder(PlayManager* playManager,
			               std::shared_ptr<PipelineExtensions::StreamRecordable> streamRecordable,
			               QObject* parent = nullptr);
			~StreamRecorder();

			void changeTrack(const MetaData& track);

			void record(bool b);
			[[nodiscard]] bool isRecording() const;

		private slots:
			void playstateChanged(PlayState state);

		private:
			bool save();
			void startNewSession();
			void endSession();
	};
}

#endif // STREAMRECORDER_H
