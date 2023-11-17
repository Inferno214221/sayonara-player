/* Engine.h */

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

#ifndef SAYONARA_PLAYBACK_ENGINE_H
#define SAYONARA_PLAYBACK_ENGINE_H

#include "Utils/typedefs.h"

#include <QObject>

#include <gst/gst.h>
#include <memory>
#include <vector>

namespace Tagging
{
	class TagWriter;
}

namespace Util
{
	class FileSystem;
}

class MetaData;

namespace Engine
{
	class LevelDataReceiver;
	class Pipeline;
	class SpectrumDataReceiver;

	class Engine :
		public QObject
	{
		Q_OBJECT

		signals:
			void sigDataAvailable(const QByteArray& data);
			void sigSpectrumChanged();
			void sigLevelChanged();

			void sigMetadataChanged(const MetaData& md);
			void sigDurationChanged(const MetaData& md);
			void sigBitrateChanged(const MetaData& md);
			void sigCoverDataAvailable(const QByteArray& data, const QString& mimetype);

			void sigCurrentPositionChanged(MilliSeconds ms);
			void sigBuffering(int progress);
			void sigTrackFinished();
			void sigTrackReady();
			void sigError(const QString& error_message);

		public:
			explicit Engine(QObject* parent);
			~Engine() noexcept override;

			Engine(const Engine& other) = delete;
			Engine(Engine&& other) = delete;
			Engine& operator=(const Engine& other) = delete;
			Engine& operator=(Engine&& other) = delete;

			virtual void updateBitrate(Bitrate br, GstElement* src) = 0;
			virtual void updateDuration(GstElement* src) = 0;

			virtual void setTrackReady(GstElement* src) = 0;
			virtual void setTrackAlmostFinished(MilliSeconds time2go) = 0;
			virtual void setTrackFinished(GstElement* src) = 0;

			[[nodiscard]] virtual bool isStreamRecorderRecording() const = 0;
			virtual void setStreamRecorderRecording(bool b) = 0;

			virtual void setSpectrum(const std::vector<float>& spectrum) = 0;
			[[nodiscard]] virtual const std::vector<float>& spectrum() const = 0;

			virtual void setLevel(float left, float right) = 0;
			[[nodiscard]] virtual QPair<float, float> level() const = 0;

			virtual void setVisualizerEnabled(bool levelEnabled, bool spectrumEnabled) = 0;
			virtual void setBroadcastEnabled(bool b) = 0;
			virtual void setEqualizer(int band, int value) = 0;

			[[nodiscard]] virtual MetaData currentTrack() const = 0;

		public slots: // NOLINT(readability-redundant-access-specifiers)
			virtual void play() = 0;
			virtual void stop() = 0;
			virtual void pause() = 0;

			virtual void jumpAbsMs(MilliSeconds ms) = 0;
			virtual void jumpRelMs(MilliSeconds ms) = 0;
			virtual void jumpRel(double percent) = 0;
			virtual void updateMetadata(const MetaData& track, GstElement* src) = 0;
			virtual void updateCover(GstElement* src, const QByteArray& data, const QString& mimedata) = 0;

			virtual bool changeTrack(const MetaData& track) = 0;

			virtual void setBufferState(int progress, GstElement* src) = 0;
			virtual void error(const QString& error, const QString& elementName) = 0;
	};

	Engine* createEngine(const std::shared_ptr<Util::FileSystem>& fileSystem,
	                             const std::shared_ptr<Tagging::TagWriter>& tagWriter,
	                             QObject* parent);
}

#endif /* SAYONARA_PLAYBACK_ENGINE_H */
