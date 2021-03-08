#include "CoverFetcherUrl.h"

#include <QString>

using Cover::Fetcher::Url;

struct Url::Private
{
	QString identifier;
	QString url;

	Private(const QString& identifier, const QString& url) :
		identifier(identifier),
		url(url) {}
};

Cover::Fetcher::Url::Url() :
	Url(QString(), QString()) {}

Url::Url(const QString& identifier, const QString& url)
{
	m = Pimpl::make<Private>(identifier, url);
}

Url::Url(const Cover::Fetcher::Url& other) :
	Url(other.identifier(), other.url()) {}

Cover::Fetcher::Url& Url::operator=(const Cover::Fetcher::Url& other)
{
	m->identifier = other.identifier();
	m->url = other.url();

	return *this;
}

Url::~Url() = default;

void Url::setIdentifier(const QString& identifier)
{
	m->identifier = identifier;
}

QString Url::identifier() const
{
	return m->identifier;
}

void Url::setUrl(const QString& url)
{
	m->url = url;
}

QString Url::url() const
{
	return m->url;
}

bool Cover::Fetcher::Url::operator==(const Url& rhs) const
{
	return (this->identifier() == rhs.identifier() && this->url() == rhs.url());
}
