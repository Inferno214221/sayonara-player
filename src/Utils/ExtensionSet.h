/* ExtensionSet.h */

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

#ifndef EXTENSIONSET_H
#define EXTENSIONSET_H

#include "Utils/Pimpl.h"

namespace Gui
{
	/**
	 * @brief Collection of extensions. Handles extensions currently
	 * active or inactive and the extension toolbar.
	 */
	class ExtensionSet
	{
		PIMPL(ExtensionSet)

		public:
			ExtensionSet();
			~ExtensionSet();
			ExtensionSet(const ExtensionSet& other);
			ExtensionSet& operator=(const ExtensionSet& other);

			void add_extension(const QString& ext, bool enabled=true);
			void remove_extension(const QString& ext);
			void clear();
			bool contains_extension(const QString& ext);
			ExtensionSet& operator<<(const QString& ext);

			void set_enabled(const QString& ext, bool b);
			void enable(const QString& ext);
			void disable(const QString& ext);

			bool has_enabled_extensions() const;
			bool has_disabled_extensions() const;

			bool is_enabled(const QString& ext) const;

			QStringList enabled_extensions() const;
			QStringList disabled_extensions() const;
			QStringList extensions() const;
	};
}

#endif // EXTENSIONSET_H
