#ifndef AMAZONCOVERFETCHER_H
#define AMAZONCOVERFETCHER_H

#include "CoverFetcherInterface.h"

namespace Cover
{
namespace Fetcher
{
	class Amazon :
            public Cover::Fetcher::Base
	{
		public:
			bool can_fetch_cover_directly() const override;
			QStringList calc_addresses_from_website(const QByteArray& website) const override;
			QString artist_address(const QString& artist) const override;
			QString album_address(const QString& artist, const QString& album) const override;
			QString search_address(const QString& str) const override;
			bool is_search_supported() const override;
			bool is_album_supported() const override;
			bool is_artist_supported() const override;
			int estimated_size() const override;
			QString keyword() const override;
		};
}
}

#endif // AMAZONCOVERFETCHER_H
