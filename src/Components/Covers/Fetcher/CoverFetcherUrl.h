#ifndef COVERFETCHERURL_H
#define COVERFETCHERURL_H

#include "Utils/Pimpl.h"

namespace Cover::Fetcher
{
	class Url
	{
		PIMPL(Url)

		public:
			Url();
			Url(bool active, const QString& identifier, const QString& url);
			Url(const Url& other);
			Url& operator=(const Url& other);
			~Url();

			void set_active(bool b);
			bool is_active() const;

			void set_identifier(const QString& identifier);
			QString identifier() const;

			void set_url(const QString& url);
			QString url() const;
	};
}

#endif // COVERFETCHERURL_H
