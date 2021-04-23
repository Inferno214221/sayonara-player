/* Cover.cpp */

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

#include "Cover.h"

Models::Cover::Cover(const QString& mimeType_, const QByteArray& imageData_) :
	mimeType(mimeType_),
	imageData(imageData_)
{}

Models::Cover::Cover() :
	Models::Cover::Cover(QString(), QByteArray())
{}

Models::Cover::MimeType Models::Cover::convertMimeType() const
{
	if(mimeType.contains("jpeg", Qt::CaseInsensitive))
	{
		return Models::Cover::MimeType::JPEG;
	}

	else if(mimeType.contains("png", Qt::CaseInsensitive))
	{
		return Models::Cover::MimeType::PNG;
	}

	return Models::Cover::MimeType::Unsupported;
}
