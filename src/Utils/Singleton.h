/* singleton.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#pragma once
#ifndef SAYONARA_SINGLETON_H
#define SAYONARA_SINGLETON_H

#define _SINGLETON_BASIC_(class_name) friend class Application; \
				protected: \
				class_name( const class_name& other ) = delete; \
				class_name& operator=(const class_name& other) = delete; \
				class_name( const class_name&& other ) = delete; \
				class_name& operator=(const class_name&& other) = delete; \
				public: \
				static class_name *instance() \
				{ static class_name instance; return &instance; } \
				virtual ~class_name ();

#define SINGLETON(class_name) _SINGLETON_BASIC_(class_name) \
				protected: \
				    class_name(); \
				private:

#define SINGLETON_QOBJECT(class_name) _SINGLETON_BASIC_(class_name) \
				protected: \
				    class_name(QObject* parent=nullptr); \
				private:


#endif // SAYONARA_SINGLETON_H
