#ifndef YANDEXCOVERFETCHER_H
#define YANDEXCOVERFETCHER_H

#include "../CoverFetcherInterface.h"

namespace Cover::Fetcher
{
	class Yandex : public Cover::Fetcher::Base
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


#endif // YANDEXCOVERFETCHER_H
