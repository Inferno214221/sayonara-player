/* LibraryNamespaces.h */

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

#ifndef LIBRARYNAMESPACES_H
#define LIBRARYNAMESPACES_H

#include <QtGlobal>

/**
 * @ingroup Library
 * @ingroup Helper
 */
namespace Library
{
	/**
	 * @brief The TrackDeletionMode enum
	 * @ingroup LibraryHelper
	 */
	enum class TrackDeletionMode :
		quint8
	{
		None = 0,
		OnlyLibrary,
		AlsoFiles
	};

	/**
	 * @brief The ReloadQuality enum
	 * @ingroup LibraryHelper
	 */
	enum class ReloadQuality :
		quint8
	{
		Fast = 0,
		Accurate,
		Unknown
	};

	enum class ViewType :
		quint8
	{
		Standard = 0,
		CoverView,
		FileView
	};
}

#endif // LIBRARYNAMESPACES_H
