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
		friend class Manager;
		PIMPL(Url)

		private:
			Url();
			Url(const QString& identifier, const QString& url);

		public:
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
