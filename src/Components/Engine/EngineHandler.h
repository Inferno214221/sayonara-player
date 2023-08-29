/* EngineHandler.h */

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

#ifndef ENGINEHANDLER_H_
#define ENGINEHANDLER_H_

#include "Utils/Pimpl.h"

#include "Interfaces/CoverDataProvider.h"
#include "Interfaces/AudioDataProvider.h"
#include "Interfaces/Engine/SoundModifier.h"

#include <QObject>

#define EngineHandler_change_track_md static_cast<void (EngineHandler::*) (const MetaData& md)>(&EngineHandler::change_track)

class PlayManager;

namespace Tagging
{
	class TagWriter;
}

namespace Util
{
	class FileSystem;
}

namespace Engine
{
	/**
	 * @brief The EngineHandler class
	 * @ingroup Engine
	 */
	class Handler :
		public QObject,
		public CoverDataProvider,
		public LevelDataProvider,
		public SpectrumDataProvider,
		public RawAudioDataProvider,
		public SoundModifier
	{
		Q_OBJECT
		PIMPL(Handler)

		public:
			Handler(const std::shared_ptr<Util::FileSystem>& fileSystem,
			        const std::shared_ptr<Tagging::TagWriter>& tagWriter,
			        PlayManager* playManager);
			~Handler() override;

			void shutdown();
			[[nodiscard]] bool isValid() const;

			void registerLevelReceiver(LevelDataReceiver* receiver) override;
			void unregisterLevelReceiver(LevelDataReceiver* levelReceiver) override;
			void levelActiveChanged(bool b) override;

			void registerSpectrumReceiver(SpectrumDataReceiver* receiver) override;
			void unregisterSpectrumReceiver(SpectrumDataReceiver* spectrumReceiver) override;
			void spectrumActiveChanged(bool b) override;

			void registerCoverReceiver(CoverDataReceiver* coverReceiver) override;
			void unregisterCoverReceiver(CoverDataReceiver* coverReceiver) override;

			void registerAudioDataReceiver(RawAudioDataReceiver* receiver) override;
			void unregisterAudioDataReceiver(RawAudioDataReceiver* receiver) override;

			void setEqualizer(int band, int value) override;

		private slots:
			void playstateChanged(PlayState state);

			void spectrumChanged();
			void levelChanged();

		private: // NOLINT(readability-redundant-access-specifiers)
			void reloadReceivers();
			void setAudioData(const QByteArray& data) override;
			void setLevelData(float left, float right) override;
			void setSpectrumData(const std::vector<float>& spectrum) override;
			void setCoverData(const QByteArray& imageData, const QString& mimeData) override;
	};
}

#endif


