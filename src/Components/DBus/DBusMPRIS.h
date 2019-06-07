/* DBusMPRIS.h */

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

#ifndef DBUS_MPRIS_H
#define DBUS_MPRIS_H

#include <QObject>
#include <QVariant>
#include <QDBusObjectPath>

#include "Components/PlayManager/PlayState.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Pimpl.h"

#include "DBusAdaptor.h"

class QMainWindow;

namespace DBusMPRIS
{

	class MediaPlayer2 :
			public DBusAdaptor
	{
		Q_OBJECT
		PIMPL(MediaPlayer2)

		public:
			explicit MediaPlayer2(QMainWindow* player, QObject *parent=nullptr);
			~MediaPlayer2();

			Q_PROPERTY(bool			CanQuit				READ CanQuit		CONSTANT)
			bool					CanQuit() const;

			Q_PROPERTY(bool			CanRaise			READ CanRaise		CONSTANT)
			bool					CanRaise();

			Q_PROPERTY(bool			HasTrackList		READ HasTrackList)
			bool					HasTrackList();


			Q_PROPERTY(QString		Identity			READ Identity		CONSTANT)
			QString					Identity();

			Q_PROPERTY(QString		DesktopEntry		READ DesktopEntry	CONSTANT)
			QString					DesktopEntry();

			Q_PROPERTY(QStringList	SupportedUriSchemes	READ SupportedUriSchemes CONSTANT)
			QStringList				SupportedUriSchemes();


			Q_PROPERTY(QStringList	SupportedMimeTypes	READ SupportedMimeTypes CONSTANT)
			QStringList				SupportedMimeTypes();


			Q_PROPERTY(bool			CanSetFullscreen	READ CanSetFullscreen)
			bool					CanSetFullscreen();

			Q_PROPERTY(bool			Fullscreen			READ Fullscreen				WRITE SetFullscreen)
			bool					Fullscreen();
			void					SetFullscreen(bool b);

			void					Raise();
			void					Quit();

		private:
			void					init();

		signals:
			void					sig_raise();
	};
} // end namespace DBusMPRIS

#endif // DBUS_MPRIS_H
