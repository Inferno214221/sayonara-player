#ifndef TAGGING_CHANGEINFORMATION_H
#define TAGGING_CHANGEINFORMATION_H

#include "Utils/Pimpl.h"

class MetaData;

namespace Tagging
{
	class ChangeInformation
	{
		PIMPL(ChangeInformation)

		public:
			ChangeInformation(const MetaData& md);
			~ChangeInformation();

			ChangeInformation(const ChangeInformation& other);
			ChangeInformation& operator=(const ChangeInformation& other);

			void update(const MetaData& md);
			void update_cover(const QPixmap& pm);

			void apply();
			void undo();

			bool has_changes() const;
			void set_changed(bool b);

			bool has_new_cover() const;
			QPixmap cover() const;

			const MetaData& current_metadata() const;
			const MetaData& original_metadata() const;

			MetaData& current_metadata();
			MetaData& original_metadata();
	};
}

#endif // TAGGING_CHANGEINFORMATION_H
