/* ActionPair.h */

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

#ifndef ACTIONPAIR_H
#define ACTIONPAIR_H

#include <QString>
#include "Utils/Library/Sortorder.h"
#include "Utils/Language/Language.h"

namespace Library
{
	class ActionPair
	{
		public:
			ActionPair(const Lang::Term term, const bool ascending, const VariableSortorder so) :
				m_term {term},
				m_ascending {ascending},
				m_sortorder {so} {}

			ActionPair(const ActionPair& other) = default;
			ActionPair& operator=(const ActionPair& other) = default;

			~ActionPair() = default;

			[[nodiscard]] QString name() const
			{
				const auto text = m_ascending
				                  ? Lang::get(Lang::Ascending)
				                  : Lang::get(Lang::Descending);

				return QString("%1 (%2)").arg(Lang::get(m_term), text);
			}

			[[nodiscard]] VariableSortorder sortorder() const { return m_sortorder; }

		private:
			Lang::Term m_term;
			bool m_ascending;
			VariableSortorder m_sortorder;
	};
}

#endif // ACTIONPAIR_H
