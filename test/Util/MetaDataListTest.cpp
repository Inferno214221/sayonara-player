#include <QObject>
#include <QTest>
#include <QDebug>

#include "Utils/FileUtils.h"
#include "Utils/RandomGenerator.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include <algorithm>

class MetaDataListTest : public QObject
{
	Q_OBJECT

private slots:
	void insert_test();
	void remove_test();
};


static MetaDataList create_v_md(int min, int max)
{
	MetaDataList v_md;
	for(int i=min; i<max; i++)
	{
		MetaData md;
		md.set_id(i);
		md.set_title(QString("title %1").arg(i));
		md.set_artist(QString("artist %1").arg(i));
		md.set_album(QString("album %1").arg(i));

		v_md << md;
	}

	return v_md;
}


void MetaDataListTest::insert_test()
{
	MetaDataList v_md = create_v_md(0, 53);
	MetaDataList inserted_md = create_v_md(100, 105);

	int insert_idx = 8;
	auto old_size = v_md.size();

	v_md.insert_tracks(inserted_md, insert_idx);
	QVERIFY(v_md.size() == old_size + inserted_md.size());
}


static Util::Set<int> create_idx_set(int n, int max_val, int ignore_val=-1)
{
	Util::Set<int> indexes;
	for(int i=0; i<n; i++){
		int idx = (i*7) % max_val;
		if(idx == ignore_val){
			continue;
		}

		indexes.insert(idx);
	}

	return indexes;
}

void MetaDataListTest::remove_test()
{
	MetaDataList v_md = create_v_md(0, 53);
	auto old_size = v_md.size();

	Util::Set<int> remove_indexes = create_idx_set(15, v_md.count());

	v_md.remove_tracks(remove_indexes);

	QVERIFY(v_md.size() == (old_size - remove_indexes.size()));
}


QTEST_GUILESS_MAIN(MetaDataListTest)

#include "MetaDataListTest.moc"

