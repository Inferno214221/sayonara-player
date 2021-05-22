#ifndef DIRECTFETCHER_H
#define DIRECTFETCHER_H

#include "CoverFetcher.h"

namespace Cover::Fetcher
{
	class DirectFetcher :
		public Cover::Fetcher::Base
	{
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
	};
}

#endif // DIRECTFETCHER_H
