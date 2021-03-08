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
			QString privateIdentifier() const override;

		public:
			DirectFetcher();
			~DirectFetcher() override;

			bool canFetchCoverDirectly() const override;
			QStringList parseAddresses(const QByteArray& website) const override;
			QString artistAddress(const QString& artist) const override;
			QString albumAddress(const QString& artist, const QString& album) const override;
			QString fulltextSearchAddress(const QString& str) const override;
			int estimatedSize() const override;
			bool isWebserviceFetcher() const override;

			void setDirectUrl(const QString& url);

	};
}

#endif // DIRECTFETCHER_H
