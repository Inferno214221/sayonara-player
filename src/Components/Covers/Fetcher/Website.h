#ifndef WEBSITE_H
#define WEBSITE_H

#include "CoverFetcher.h"
#include "Utils/Pimpl.h"

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
			QString priv_identifier() const override;

		public:
			Website();
			~Website() override;

			bool can_fetch_cover_directly() const override;
			QStringList parse_addresses(const QByteArray& website) const override;

			int estimated_size() const override;

			/**
			 * @brief will always return the website which has been set by
			 * set_website(const QString&)
			 * @param address ignored
			 * @return
			 */
			QString search_address(const QString& address) const override;

			void set_website(const QString& website);
    };
}

#endif // WEBSITE_H
