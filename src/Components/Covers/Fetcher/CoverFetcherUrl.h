/* CoverFetcherUrl.h
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef COVERFETCHERURL_H
#define COVERFETCHERURL_H

#include "Utils/Pimpl.h"

namespace Cover::Fetcher
{
	class Manager;
	/**
	 * @brief An Url is defined by its identifier and a custom url string.
	 * The identifier is the same as being used in the Cover::Fetcher::Base
	 * classes. The url is a standard string you also can enter in your
	 * browser. The identifier can be used to ask its active status
	 * in the Cover::Fetcher::Manager for example
	 */
	class Url
	{
		PIMPL(Url)

		private:
			Url();

		public:
			Url(const QString& identifier, const QString& url);
			Url(const Url& other);
			Url& operator=(const Url& other);
			~Url();

			void setIdentifier(const QString& identifier);
			QString identifier() const;

			void setUrl(const QString& url);
			QString url() const;
			bool operator==(const Url& rhs) const;
	};
}

#endif // COVERFETCHERURL_H
