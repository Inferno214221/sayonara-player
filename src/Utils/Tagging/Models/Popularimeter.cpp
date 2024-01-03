/* Popularimeter.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

Models::Popularimeter::Popularimeter() :
	Models::Popularimeter::Popularimeter("sayonara player", Rating::Zero, 0) {}

Models::Popularimeter::Popularimeter(const QString& email, Rating rating, int playcount) :
	email(email),
	rating(rating),
	playcount(playcount) {}

void Models::Popularimeter::setRatingByte(Byte byte)
{
	if(byte == 0x00)
	{
		rating = Rating::Zero;
	}
	else if(byte < 0x30)
	{   //48
		rating = Rating::One;
	}
	else if(byte < 0x60)
	{    // 92
		rating = Rating::Two;
	}
	else if(byte < 0xA0)
	{    // 160
		rating = Rating::Three;
	}
	else if(byte < 0xD8)
	{    // 216
		rating = Rating::Four;
	}
	else
	{
		rating = Rating::Five;            // 255
	}
}

Byte Models::Popularimeter::ratingByte() const
{
	Byte rating_byte;

	switch(rating)
	{
		case Rating::Zero:
			rating_byte = 0x00;
			break;
		case Rating::One:
			rating_byte = 0x01; // 1
			break;
		case Rating::Two:
			rating_byte = 0x40; // 64
			break;
		case Rating::Three:
			rating_byte = 0x7F; // 128
			break;
		case Rating::Four:
			rating_byte = 0xC0; // 192
			break;
		case Rating::Five:
			rating_byte = 0xFF; // 255
			break;
		default:
			rating_byte = 0x00;
	}

	return rating_byte;
}

QString Models::Popularimeter::toString()
{
	return QString("POPM: %1 %2 %3")
		.arg(email)
		.arg(ratingByte())
		.arg(playcount);
}
