#include "CoverFetcherUrl.h"

#include <QString>

using Cover::Fetcher::Url;

struct Url::Private
{
	bool active;
	QString identifier;
	QString url;

	Private() :
		active(false)
	{}
};

Cover::Fetcher::Url::Url()
{
	m = Pimpl::make<Private>();
}

Url::Url(bool active, const QString& identifier, const QString& url) :
	Url()
{
	m->active = active;
	m->identifier = identifier;
	m->url = url;
}

Url::Url(const Cover::Fetcher::Url& other) :
	Url(other.is_active(), other.identifier(), other.url())
{}

Cover::Fetcher::Url& Url::operator=(const Cover::Fetcher::Url& other)
{
	m->active = other.is_active();
	m->identifier = other.identifier();
	m->url = other.url();

	return *this;
}

Url::~Url() = default;

void Url::set_active(bool b)
{
	m->active = b;
}

bool Url::is_active() const
{
	return m->active;
}

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
