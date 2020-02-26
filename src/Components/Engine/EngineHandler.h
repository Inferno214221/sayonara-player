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

#include "Components/PlayManager/PlayState.h"
#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

#include <QObject>

#define EngineHandler_change_track_md static_cast<void (EngineHandler::*) (const MetaData& md)>(&EngineHandler::change_track)

namespace Engine
{
	class RawSoundReceiverInterface;
	class LevelReceiver;
	class SpectrumReceiver;

	/**
	 * @brief The EngineHandler class
	 * @ingroup Engine
	 */
	class Handler :
			public QObject
	{
		Q_OBJECT
		SINGLETON_QOBJECT(Handler)
		PIMPL(Handler)

		signals:
			void sigCoverDataAvailable(const QByteArray& data, const QString& mimetype);

		public:
			void shutdown();
			bool isValid() const;

			void registerRawSoundReceiver(RawSoundReceiverInterface* receiver);
			void unregisterRawSoundReceiver(RawSoundReceiverInterface* receiver);

			void registerLevelReceiver(LevelReceiver* receiver);
			void registerSpectrumReceiver(SpectrumReceiver* receiver);

			void setEqualizer(int band, int value);

		public slots:
			void reloadLevelReceivers();
			void reloadSpectrumReceivers();

		private slots:
			void playstateChanged(PlayState state);
			void newAudioDataAvailable(const QByteArray& data);
			void spectrumChanged();
			void levelChanged();

		private:
			void reloadReceivers();
	};
}

#endif


