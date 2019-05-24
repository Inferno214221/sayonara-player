/* EngineHandler.h */

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

#ifndef ENGINEHANDLER_H_
#define ENGINEHANDLER_H_

#include "Components/PlayManager/PlayState.h"
#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

#include <QObject>

#define EngineHandler_change_track_md static_cast<void (EngineHandler::*) (const MetaData& md)>(&EngineHandler::change_track)

class RawSoundReceiverInterface;
class LevelReceiver;
class SpectrumReceiver;


/**
 * @brief The EngineHandler class
 * @ingroup Engine
 */
class EngineHandler :
		public QObject
{
	Q_OBJECT
	SINGLETON_QOBJECT(EngineHandler)
	PIMPL(EngineHandler)

	signals:
		void sig_md_changed(const MetaData& md);
		void sig_duration_changed(const MetaData& md);
		void sig_bitrate_changed(const MetaData& md);
		void sig_cover_changed(const QImage& img);

	public:
		bool init();
		void shutdown();

		void register_raw_sound_receiver(RawSoundReceiverInterface* receiver);
		void unregister_raw_sound_receiver(RawSoundReceiverInterface* receiver);

		void add_level_receiver(LevelReceiver* receiver);
		void add_spectrum_receiver(SpectrumReceiver* receiver);

		void set_equalizer(int band, int value);

	private slots:
		void playstate_changed(PlayState state);

		void new_data(const uchar* data, uint64_t n_bytes);
};

#endif


