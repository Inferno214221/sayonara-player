#ifndef STOPBEHAVIOR_H
#define STOPBEHAVIOR_H

#include "Utils/Pimpl.h"

class MetaDataList;
namespace Playlist
{
	class StopBehavior
	{
		PIMPL(StopBehavior)

	public:
		StopBehavior();
		~StopBehavior();

		virtual const MetaDataList& metadata() const=0;

		int restore_track_before_stop();

		int track_idx_before_stop() const;
		void set_track_idx_before_stop(int idx);
	};
}

#endif // STOPBEHAVIOR_H
