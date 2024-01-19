/* CoverViewSortorderInfo.cpp */

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

#include "CoverViewSortorderInfo.h"

namespace Library
{
	QString CoverViewSortorderInfo::name() const
	{
		const auto text = ascending
		                  ? Lang::get(Lang::Ascending)
		                  : Lang::get(Lang::Descending);

		return QString("%1 (%2)").arg(Lang::get(term), text);
	}
}