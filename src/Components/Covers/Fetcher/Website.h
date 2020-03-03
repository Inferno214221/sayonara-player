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
			QString privateIdentifier() const override;

		public:
			Website();
			~Website() override;

			bool canFetchCoverDirectly() const override;
			QStringList parseAddresses(const QByteArray& website) const override;

			int estimatedSize() const override;

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
