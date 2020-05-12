#ifndef SOMAFMUTILS_H
#define SOMAFMUTILS_H

class MetaDataList;

namespace SomaFM
{
	class Station;
	namespace Utils
	{
		void mapStationToMetadata(const SomaFM::Station& station, MetaDataList& tracks);
	}
}

#endif // SOMAFMUTILS_H
