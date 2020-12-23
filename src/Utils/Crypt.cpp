/* Crypt.cpp */

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

#include "Crypt.h"
#include "Utils/Settings/Settings.h"

namespace
{
	QByteArray encryptData(const QByteArray& src, QByteArray key)
	{
		if(src.isEmpty())
		{
			return QByteArray();
		}

		if(key.isEmpty())
		{
			key = GetSetting(Set::Player_PrivId);
		}

		QByteArray result;
		for(auto i = 0; i < src.length(); i++)
		{
			const auto currentByte = *(src.data() + i);
			const auto pos = i % key.size();
			const auto maskChar = *(key.data() + pos);
			const auto resultChar = currentByte ^maskChar;

			result.push_back(resultChar);
		}

		return result;
	}
}

QString Util::Crypt::encrypt(const QString& src, const QByteArray& key)
{
	const auto encryptedData = encryptData(src.toUtf8(), key);
	return SettingConverter::toString(encryptedData);
}

QString Util::Crypt::encrypt(const QByteArray& src, const QByteArray& key)
{
	const auto encryptedData = encryptData(src, key);
	return SettingConverter::toString(encryptedData);
}

QString Util::Crypt::decrypt(const QString& src, const QByteArray& key)
{
	QByteArray sourceData;
	SettingConverter::fromString(src, sourceData);

	const auto decryptedData = encryptData(sourceData, key);
	return QString::fromUtf8(decryptedData);
}

QString Util::Crypt::decrypt(const QByteArray& src, const QByteArray& key)
{
	return encryptData(src, key);
}
