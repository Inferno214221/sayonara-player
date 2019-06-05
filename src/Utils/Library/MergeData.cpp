#include "MergeData.h"
#include "Utils/Set.h"

struct MergeData::Private
{
	Util::Set<Id>	source_ids;
	Id				target_id;
	LibraryId		library_id;

	Private(const Util::Set<Id>& source_ids, Id target_id, LibraryId library_id) :
		source_ids(source_ids),
		target_id(target_id),
		library_id(library_id)
	{}
};

MergeData::MergeData(const Util::Set<Id>& source_ids, Id target_id, LibraryId library_id)
{
	m = Pimpl::make<Private>(source_ids, target_id, library_id);
}

MergeData::MergeData(const MergeData& other)
{
	m = Pimpl::make<Private>(other.source_ids(), other.target_id(), other.library_id());
}

MergeData::~MergeData() = default;

MergeData& MergeData::operator=(const MergeData& other)
{
	*m = *(other.m);
	return *this;
}

bool MergeData::is_valid() const
{
	return ((target_id() >= 0) && (source_ids().count() >= 2) && !(source_ids().contains(-1)));
}

Util::Set<Id> MergeData::source_ids() const
{
	return m->source_ids;
}

Id MergeData::target_id() const
{
	return m->target_id;
}

LibraryId MergeData::library_id() const
{
	return m->library_id;
}
