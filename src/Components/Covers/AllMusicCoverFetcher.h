#ifndef ALLMUSICCOVERFETCHER_H
#define ALLMUSICCOVERFETCHER_H

#include "CoverFetcherInterface.h"

namespace Cover
{
namespace Fetcher
{

	class AllMusic :
		public Cover::Fetcher::Base
	{
		private:
			QString priv_identifier() const override;

		public:
			bool can_fetch_cover_directly() const override;
			QStringList parse_addresses(const QByteArray& website) const override;

			QString artist_address(const QString& artist) const override;
			QString album_address(const QString& artist, const QString& album) const override;
			QString search_address(const QString& str) const override;

			int estimated_size() const override;
	};
}
}

#endif // ALLMUSICCOVERFETCHER_H
