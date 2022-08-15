/* DBusMPRIS.h */

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

#ifndef DBUS_MPRIS_H
#define DBUS_MPRIS_H

#include "DBusAdaptor.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Pimpl.h"

#include <QObject>
#include <QVariant>
#include <QDBusObjectPath>

using QStrRef = const QString&;

class QMainWindow;
class PlayManager;
class PlaylistAccessor;

namespace DBusMPRIS
{
	class MediaPlayer2 :
		public DBusAdaptor
	{
		Q_OBJECT
		PIMPL(MediaPlayer2)

		signals:
			void Seeked(qlonglong position);

		public:
			explicit MediaPlayer2(QMainWindow* player, PlayManager* playManager, PlaylistAccessor* playlistAccessor,
			                      QObject* parent = nullptr);
			~MediaPlayer2() override;

			Q_PROPERTY(bool CanQuit READ CanQuit CONSTANT)
			[[nodiscard]] bool CanQuit() const;

			Q_PROPERTY(bool CanRaise READ CanRaise CONSTANT)
			bool CanRaise();

			Q_PROPERTY(bool HasTrackList READ HasTrackList)
			bool HasTrackList();

			Q_PROPERTY(QString Identity READ Identity CONSTANT)
			QString Identity();

			Q_PROPERTY(QString DesktopEntry READ DesktopEntry CONSTANT)
			QString DesktopEntry();

			Q_PROPERTY(QStringList SupportedUriSchemes READ SupportedUriSchemes CONSTANT)
			QStringList SupportedUriSchemes();

			Q_PROPERTY(QStringList SupportedMimeTypes READ SupportedMimeTypes CONSTANT)
			QStringList SupportedMimeTypes();

			Q_PROPERTY(bool CanSetFullscreen READ CanSetFullscreen)
			bool CanSetFullscreen();

			Q_PROPERTY(bool Fullscreen READ Fullscreen WRITE SetFullscreen)
			bool Fullscreen();
			void SetFullscreen(bool b);

			[[maybe_unused]] void Raise();
			[[maybe_unused]] void Quit();

			Q_PROPERTY(QString PlaybackStatus READ PlaybackStatus)
			QString PlaybackStatus();

			Q_PROPERTY(QString LoopStatus READ LoopStatus WRITE SetLoopStatus)
			QString LoopStatus();
			void SetLoopStatus(QString status);

			Q_PROPERTY(double Rate READ Rate WRITE SetRate)
			double Rate();
			void SetRate(double rate);

			Q_PROPERTY(int Rating READ Rating)
			int Rating();

			Q_PROPERTY(bool Shuffle READ Shuffle WRITE SetShuffle)
			bool Shuffle();
			void SetShuffle(bool shuffle);

			Q_PROPERTY(QVariantMap Metadata READ Metadata)
			QVariantMap Metadata();

			Q_PROPERTY(double Volume READ Volume WRITE SetVolume)
			double Volume();
			void SetVolume(double volume);
			[[maybe_unused]] void IncreaseVolume();
			[[maybe_unused]] void DecreaseVolume();

			Q_PROPERTY(qlonglong Position READ Position)
			qlonglong Position();
			[[maybe_unused]] void SetPosition(const QDBusObjectPath& trackId, qlonglong position);

			Q_PROPERTY(double MinimumRate READ MinimumRate)
			double MinimumRate();

			Q_PROPERTY(double MaximumRate READ MaximumRate)
			double MaximumRate();

			Q_PROPERTY(bool CanGoNext READ CanGoNext)
			bool CanGoNext();

			Q_PROPERTY(bool CanGoPrevious READ CanGoPrevious)
			bool CanGoPrevious();

			Q_PROPERTY(bool CanPlay READ CanPlay)
			bool CanPlay();

			Q_PROPERTY(bool CanPause READ CanPause)
			bool CanPause();

			Q_PROPERTY(bool CanSeek READ CanSeek)
			bool CanSeek();

			Q_PROPERTY(bool CanControl READ CanControl)
			bool CanControl();

			void Next();
			[[maybe_unused]] void Previous();
			[[maybe_unused]] void Pause();
			[[maybe_unused]] void PlayPause();
			void Stop();
			void Play();
			[[maybe_unused]] void Seek(qlonglong offset);
			[[maybe_unused]] void OpenUri(const QString& uri);

		public slots: // NOLINT(readability-redundant-access-specifiers)
			void positionChanged(MilliSeconds pos_ms);
			void volumeChanged(int volume);
			void trackIndexChanged(int idx);
			void trackChanged(const MetaData& track);
			void playstateChanged(PlayState state);

		private: // NOLINT(readability-redundant-access-specifiers)
			void init();
	};
} // end namespace DBusMPRIS

#endif // DBUS_MPRIS_H
