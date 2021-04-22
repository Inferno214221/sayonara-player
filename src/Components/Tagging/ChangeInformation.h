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
			ChangeInformation(const MetaData& track);
			~ChangeInformation();

			ChangeInformation(const ChangeInformation& other);
			ChangeInformation& operator=(const ChangeInformation& other);

			void update(const MetaData& track);
			void updateCover(const QPixmap& pm);

			/**
			 * @brief Overwrite original track with the modified one.
			 * This cannot be undone
			 */
			void apply();

			/**
			 * @brief Overwrite modified track with the original one.
			 */
			void undo();

			bool hasChanges() const;
			void setChanged(bool b);

			bool hasNewCover() const;
			QPixmap cover() const;

			const MetaData& currentMetadata() const;
			const MetaData& originalMetadata() const;

			MetaData& currentMetadata();
			MetaData& originalMetadata();
	};
}

#endif // TAGGING_CHANGEINFORMATION_H
