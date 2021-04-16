/* Popularimeter.h */

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

#ifndef SAYONARA_TAGGING_POPULARIMETER_MODEL_H
#define SAYONARA_TAGGING_POPULARIMETER_MODEL_H

#include "Utils/typedefs.h"

#include <QString>

namespace Models
{
	/**
	 * @brief The Popularimeter class
	 * @ingroup Tagging
	 */
	struct Popularimeter
	{
		QString		email;
		Rating		rating;
		int			playcount;

		Popularimeter();
		Popularimeter(const QString& email, Rating rating_byte, int playcount);
		void set_rating(Rating max_5);
		void set_rating_byte(Byte byte);
		Rating get_rating() const;
		Byte get_rating_byte() const;
		QString to_string();
	};
}

#endif // SAYONARA_TAGGING_POPULARIMETER_MODEL_H
