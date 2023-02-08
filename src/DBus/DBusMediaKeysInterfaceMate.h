/* DBusMediaKeysInterfaceMate.h */

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

#ifndef DBUSMEDIAKEYSINTERFACEMATE_H
#define DBUSMEDIAKEYSINTERFACEMATE_H

#include "DBusMediaKeysInterface.h"

namespace Dbus
{
	class MediaKeysInterfaceMate :
		public MediaKeysInterface
	{
		Q_OBJECT
		PIMPL(MediaKeysInterfaceMate)

		public:
			explicit MediaKeysInterfaceMate(PlayManager* playManager, QObject* parent = nullptr);
			~MediaKeysInterfaceMate() override;

		protected:
			QString serviceName() const override;
			QDBusPendingReply<> grabMediaKeyReply() override;
			void connectMediaKeys() override;
	};
}

#endif // DBUSMEDIAKEYSINTERFACEMATE_H
