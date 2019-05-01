/* Popularimeter.cpp */

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

#include "Popularimeter.h"

Models::Popularimeter::Popularimeter()
{
	email = "sayonara player";
	rating = 0;
	playcount = 0;
}

Models::Popularimeter::Popularimeter(const QString& email, Rating rating, int playcount) :
	email(email),
	rating(rating),
	playcount(playcount)
{}


void Models::Popularimeter::set_rating(Rating max_5)
{
	rating = max_5;
}

void Models::Popularimeter::set_rating_byte(Byte byte)
{
	if(byte == 0x00){
		rating = 0;
	}
	else if(byte < 0x30){   //48
		rating = 1;
	}
	else if(byte < 0x60){	// 92
		rating = 2;
	}
	else if(byte < 0xA0){	// 160
		rating = 3;
	}
	else if(byte < 0xD8){	// 216
		rating = 4;
	}
	else{
		rating = 5;			// 255
	}
}

Rating Models::Popularimeter::get_rating() const
{
	return rating;
}

Byte Models::Popularimeter::get_rating_byte() const
{
	Byte rating_byte;

	switch(rating)
	{
		case 0:
			rating_byte = 0x00;
			break;
		case 1:
			rating_byte = 0x01; // 1
			break;
		case 2:
			rating_byte = 0x40; // 64
			break;
		case 3:
			rating_byte = 0x7F; // 128
			break;
		case 4:
			rating_byte = 0xC0; // 192
			break;
		default:
			rating_byte = 0xFF; // 255
	}

	return rating_byte;
}

QString Models::Popularimeter::to_string()
{
	return QString("POPM: ") + email + " " + QString::number(get_rating_byte()) + " " + QString::number(playcount);
}
