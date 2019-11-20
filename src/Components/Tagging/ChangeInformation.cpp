#include "ChangeInformation.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Logger/Logger.h"
#include <QPixmap>

using Tagging::ChangeInformation;

struct ChangeInformation::Private
{
	MetaData md_orig;
	MetaData md_changed;

	QPixmap new_cover;

	bool has_changes;
	bool has_new_cover;

	Private(const MetaData& md) :
		md_orig(md),
		md_changed(md),
		has_changes(false),
		has_new_cover(false)
	{}

	Private(const Private& other) :
		CASSIGN(md_orig),
		CASSIGN(md_changed),
		CASSIGN(new_cover),
		CASSIGN(has_changes),
		CASSIGN(has_new_cover)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(md_orig);
		ASSIGN(md_changed);
		ASSIGN(new_cover);
		ASSIGN(has_changes);
		ASSIGN(has_new_cover);

		return *this;
	}
};

ChangeInformation::ChangeInformation(const MetaData& md)
{
	m = Pimpl::make<Private>(md);
}

ChangeInformation::~ChangeInformation() = default;

Tagging::ChangeInformation::ChangeInformation(const Tagging::ChangeInformation& other)
{
	m = Pimpl::make<Private>(*other.m);
}

Tagging::ChangeInformation& ChangeInformation::operator=(const Tagging::ChangeInformation& other)
{
	*m = *(other.m);
	return *this;
}

void ChangeInformation::update(const MetaData& md)
{
	bool is_equal = md.is_equal_deep( m->md_orig );
	if(!is_equal)
	{
		m->md_changed = md;
		m->has_changes = true;
	}
}

void ChangeInformation::update_cover(const QPixmap& pm)
{
	if(pm.isNull()){
		sp_log(Log::Warning, this) << "Bad cover: Will not update";
		return;
	}

	m->new_cover = pm;
	m->has_new_cover = true;
}

void ChangeInformation::apply()
{
	m->md_orig = m->md_changed;
	m->has_changes = false;
	m->has_new_cover = false;
}

void ChangeInformation::undo()
{
	m->has_changes = false;
	m->md_changed = m->md_orig;
	m->new_cover = QPixmap();
	m->has_new_cover = false;
}

bool ChangeInformation::has_changes() const
{
	return m->has_changes;
}

void ChangeInformation::set_changed(bool b)
{
	m->has_changes = b;
}

bool ChangeInformation::has_new_cover() const
{
	return m->has_new_cover;
}

QPixmap ChangeInformation::cover() const
{
	return m->new_cover;
}

MetaData& ChangeInformation::current_metadata()
{
	return m->md_changed;
}

MetaData& ChangeInformation::original_metadata()
{
	return m->md_orig;
}

const MetaData& ChangeInformation::current_metadata() const
{
	return m->md_changed;
}

const MetaData& ChangeInformation::original_metadata() const
{
	return m->md_orig;
}
