#include "CoverFetcherUrl.h"

#include <QString>

using Cover::Fetcher::Url;

struct Url::Private
{
	QString identifier;
	QString url;
};

Cover::Fetcher::Url::Url()
{
	m = Pimpl::make<Private>();
}

Url::Url(const QString& identifier, const QString& url) :
	Url()
{
	m->identifier = identifier;
	m->url = url;
}

Url::Url(const Cover::Fetcher::Url& other) :
	Url(other.identifier(), other.url())
{}

Cover::Fetcher::Url& Url::operator=(const Cover::Fetcher::Url& other)
{
	m->identifier = other.identifier();
	m->url = other.url();

	return *this;
}

Url::~Url() = default;

void Url::set_identifier(const QString& identifier)
{
	m->identifier = identifier;
}

QString Url::identifier() const
{
	return m->identifier;
}

void Url::set_url(const QString& url)
{
	m->url = url;
}

QString Url::url() const
{
	return m->url;
}
