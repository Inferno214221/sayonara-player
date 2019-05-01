/* PreferenceRegistry.h */

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



#ifndef PREFERENCEREGISTRY_H
#define PREFERENCEREGISTRY_H

#include "Utils/Pimpl.h"
#include "Utils/Singleton.h"

class PreferenceUi
{
	public:
		virtual void show_preference(const QString& name)=0;
};

class PreferenceRegistry
{
	PIMPL(PreferenceRegistry)
	SINGLETON(PreferenceRegistry)

	public:
		void register_preference(const QString& name);
		void set_user_interface(PreferenceUi* ui);
		void show_preference(const QString& name);
};

#endif // PREFERENCEREGISTRY_H
