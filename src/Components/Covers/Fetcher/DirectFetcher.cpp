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

QString DirectFetcher::priv_identifier() const
{
	return "direct";
}

bool DirectFetcher::can_fetch_cover_directly() const
{
	return true;
}

QStringList DirectFetcher::parse_addresses(const QByteArray& website) const
{
	Q_UNUSED(website);
	return QStringList();
}

QString DirectFetcher::artist_address(const QString& artist) const
{
	Q_UNUSED(artist)

	return m->url;
}

QString DirectFetcher::album_address(const QString& artist, const QString& album) const
{
	Q_UNUSED(artist)
	Q_UNUSED(album)

	return m->url;
}

QString DirectFetcher::search_address(const QString& str) const
{
	Q_UNUSED(str)

	return m->url;
}

int DirectFetcher::estimated_size() const
{
	return 1;
}

void Cover::Fetcher::DirectFetcher::set_direct_url(const QString& url)
{
	m->url = url;
}

