#ifndef ALLMUSICCOVERFETCHER_H
#define ALLMUSICCOVERFETCHER_H

#include "CoverFetcherInterface.h"

namespace Cover
{
namespace Fetcher
{

	class AllMusicCoverFetcher :
		public Cover::Fetcher::Base
	{
		public:
			bool can_fetch_cover_directly() const;
			QStringList calc_addresses_from_website(const QByteArray& website) const;
			QString identifier() const;
			QString artist_address(const QString& artist) const;
			QString album_address(const QString& artist, const QString& album) const;
			QString search_address(const QString& str) const;
			bool is_search_supported() const;
			bool is_album_supported() const;
			bool is_artist_supported() const;
			int estimated_size() const;
	};
}
}

#endif // ALLMUSICCOVERFETCHER_H
