#ifndef DIRECTFETCHER_H
#define DIRECTFETCHER_H

#include "CoverFetcher.h"
#include "Utils/Pimpl.h"

namespace Cover::Fetcher
{
	class DirectFetcher :
		public Cover::Fetcher::Base
	{
		PIMPL(DirectFetcher)

		private:
			QString priv_identifier() const override;

		public:
			DirectFetcher();
			~DirectFetcher() override;

			bool can_fetch_cover_directly() const override;
			QStringList parse_addresses(const QByteArray& website) const override;
			QString artist_address(const QString& artist) const override;
			QString album_address(const QString& artist, const QString& album) const override;
			QString search_address(const QString& str) const override;
			int estimated_size() const override;

			void set_direct_url(const QString& url);
	};
}

#endif // DIRECTFETCHER_H
