/* Set.h */

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

#ifndef SET_H
#define SET_H

#include <set>
#include <QList>

namespace Util
{
	template<typename T>

	/**
	 * @brief A set structure. Inherited from std::set with some useful methods. For integer and String this set is ordered
	 * @ingroup Helper
	 */
	class Set :
		public std::set<T>
	{
		public:
			Set() :
				std::set<T>() {}

			/**
			 * @brief Constructs a set with a single element
			 * @param singleElement the first element
			 */
			Set(const T& singleElement) :
				Set()
			{
				this->insert(singleElement);
			}

			/**
			 * @brief converts the set to a list. The order is random
			 * @return list
			 */
			QList<T> toList() const
			{
				QList<T> ret;
				for(auto it = this->cbegin(); it != this->cend(); it++)
				{
					ret << *it;
				}
				return ret;
			}

			/**
			 * @brief
			 * @return true, if set is empty
			 */
			bool isEmpty() const
			{
				return (this->size() == 0);
			}

			/**
			 * @brief get copy of first element
			 * @return first element
			 */
			T first() const
			{
				return *(this->begin());
			}

			/**
			 * @brief check, if set contains a specific value
			 * @param value
			 * @return
			 */
			bool contains(const T& value) const
			{
				auto it = this->find(value);
				return (it != this->end());
			}

			/**
			 * @brief removes every item that matches value
			 * @param value
			 */
			void remove(const T& value)
			{
				auto it = this->find(value);
				if(it != this->end())
				{
					this->erase(it);
				}
			}

			Util::Set<T>& operator<<(const T& t)
			{
				this->insert(t);
				return *this;
			}

			template<template<typename> class A>
			Util::Set<T>& operator<<(const A<T>& container)
			{
				for(const T& t: container)
				{
					this->insert(t);
				}

				return *this;
			}

			int count() const
			{
				return static_cast<int>(this->size());
			}
	};
}

#endif // SET_H
