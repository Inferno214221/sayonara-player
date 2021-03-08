#include "DirectFetcher.h"
#include <QStringList>

using Cover::Fetcher::DirectFetcher;

struct DirectFetcher::Private
{
	QString url;
};

DirectFetcher::DirectFetcher() :
	Cover::Fetcher::Base()
{
	m = Pimpl::make<Private>();
}

DirectFetcher::~DirectFetcher() = default;

QString DirectFetcher::privateIdentifier() const
{
	return "direct";
}

bool DirectFetcher::canFetchCoverDirectly() const
{
	return true;
}

QStringList DirectFetcher::parseAddresses([[maybe_unused]] const QByteArray& website) const
{
	return QStringList();
}

QString DirectFetcher::artistAddress([[maybe_unused]] const QString& artist) const
{
	return m->url;
}

QString DirectFetcher::albumAddress([[maybe_unused]] const QString& artist, [[maybe_unused]] const QString& album) const
{
	return m->url;
}

QString DirectFetcher::fulltextSearchAddress([[maybe_unused]] const QString& str) const
{
	return m->url;
}

int DirectFetcher::estimatedSize() const
{
	return 1;
}

void Cover::Fetcher::DirectFetcher::setDirectUrl(const QString& url)
{
	m->url = url;
}

bool Cover::Fetcher::DirectFetcher::isWebserviceFetcher() const
{
	return false;
}

