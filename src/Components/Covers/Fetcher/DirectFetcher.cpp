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

QStringList DirectFetcher::parseAddresses(const QByteArray& website) const
{
	Q_UNUSED(website)
	return QStringList();
}

QString DirectFetcher::artistAddress(const QString& artist) const
{
	Q_UNUSED(artist)

	return m->url;
}

QString DirectFetcher::albumAddress(const QString& artist, const QString& album) const
{
	Q_UNUSED(artist)
	Q_UNUSED(album)

	return m->url;
}

QString DirectFetcher::fulltextSearchAddress(const QString& str) const
{
	Q_UNUSED(str)

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

