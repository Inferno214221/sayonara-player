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

Models::Cover::Cover()
{
	description = "Cover by Sayonara Player";
}

Models::Cover::Cover(const QString& mime_type_, const QByteArray& image_data_) :
	Models::Cover::Cover()
{
	mimeType = mime_type_;
	imageData = image_data_;
}

Models::Cover::MimeType Models::Cover::getMimeType() const
{
	if(mimeType.contains("jpeg", Qt::CaseInsensitive)){
		return Models::Cover::MimeType::JPEG;
	}

	else if(mimeType.contains("png", Qt::CaseInsensitive)){
		return Models::Cover::MimeType::PNG;
	}

	return Models::Cover::MimeType::Unsupported;
}
