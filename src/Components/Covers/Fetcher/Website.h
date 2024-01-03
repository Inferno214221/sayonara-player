/* Website.h
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

#ifndef WEBSITE_H
#define WEBSITE_H

#include "CoverFetcher.h"
#include "Utils/Pimpl.h"

#include <QString>

namespace Cover::Fetcher
{
	/**
	 * @brief Parses a website for all images. This cover fetcher behaves
	 * different from the others because every Cover::Fetcher::Base has
	 * a special website attached to it. This one does not have a special
	 * website. The search address will return the input itself because
	 * when searching for "https://kexp.org" this is exactly the search
	 * address.
	 * @ingroup Covers
	 */
	class Website :
		public Cover::Fetcher::Base
	{
		PIMPL(Website)

		private:
			QString privateIdentifier() const override;

		public:
			Website(const QString& url=QString());
			~Website() override;

			bool canFetchCoverDirectly() const override;
			QStringList parseAddresses(const QByteArray& website) const override;

			int estimatedSize() const override;
			bool isWebserviceFetcher() const override;

			/**
			 * @brief will always return the website which has been set by
			 * set_website(const QString&)
			 * @param address ignored
			 * @return
			 */
			QString fulltextSearchAddress(const QString& address) const override;

			virtual void setWebsite(const QString& website);
	};
}

#endif // WEBSITE_H
