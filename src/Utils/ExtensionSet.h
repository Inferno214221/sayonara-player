/* ExtensionSet.h */

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

			void addExtension(const QString& ext, bool enabled = true);
			void removeExtension(const QString& ext);
			void clear();
			bool containsExtension(const QString& ext);
			ExtensionSet& operator<<(const QString& ext);

			void setEnabled(const QString& ext, bool b);
			void enable(const QString& ext);
			void disable(const QString& ext);

			bool hasEnabledExtensions() const;
			bool hasDisabledExtensions() const;

			bool isEnabled(const QString& ext) const;

			QStringList enabledExtensions() const;
			QStringList disabledExtensions() const;
			QStringList extensions() const;
	};
}

#endif // EXTENSIONSET_H
