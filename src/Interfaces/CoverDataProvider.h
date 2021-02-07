/* ${CLASS_NAME}.h */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_COVERIMAGEPROVIDER_H
#define SAYONARA_PLAYER_COVERIMAGEPROVIDER_H

class QByteArray;
class QString;

class CoverDataReceiver;
class CoverImageProvider
{
	public:
		virtual ~CoverImageProvider() = default;

		virtual void setCoverData(const QByteArray& imageData, const QString& mimeData)=0;

		virtual void registerCoverReceiver(CoverDataReceiver* coverDataReceiver)=0;
		virtual void unregisterCoverReceiver(CoverDataReceiver* coverDataReceiver)=0;
};

#endif //SAYONARA_PLAYER_COVERIMAGEPROVIDER_H
