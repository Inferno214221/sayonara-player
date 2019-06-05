#ifndef MERGEDATA_H
#define MERGEDATA_H

#include "Utils/Pimpl.h"
#include "Utils/SetFwd.h"
#include "Utils/typedefs.h"

class MergeData
{
	PIMPL(MergeData)

	public:
		MergeData(const Util::Set<Id>& source_ids, Id target_id, LibraryId library_id);
		MergeData(const MergeData& other);
		~MergeData();

		MergeData& operator=(const MergeData& other);
		bool			is_valid() const;
		Util::Set<Id>	source_ids() const;
		Id				target_id() const;
		LibraryId		library_id() const;
};

#endif // MERGEDATA_H
