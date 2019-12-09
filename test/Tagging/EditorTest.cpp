#include "SayonaraTest.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Components/Tagging/Editor.h"
#include "Components/Tagging/ChangeNotifier.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Tagging/Tagging.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include <QFile>
#include <QSignalSpy>

using namespace Tagging;

class EditorTest :
	public Test::Base
{
	Q_OBJECT

public:
	EditorTest();
	~EditorTest()=default;

	MetaDataList create_metadata(int artists, int albums, int tracks);

private slots:
	void test_init();
	void test_rating();
	void test_one_album();
	void test_changed_metadata();
	void test_edit();
	void test_cover();
	void test_commit();
};

EditorTest::EditorTest() :
	Test::Base("EditorTest")
{
	auto* db = DB::Connector::instance();
	db->register_library_db(0);

	auto* lib_db = db->library_db(0, DbId(0));
	lib_db->store_metadata(create_metadata(2, 2, 10));

	db->close_db();
}

MetaDataList EditorTest::create_metadata(int artists, int albums, int tracks)
{
	QStringList genres[]
	{
		{"Metal", "Folk", "Blues"},
		{"Pop"},
		{""}
	};

	TrackID track_id = 1;

	MetaDataList v_md;
	for(int ar=0; ar<artists; ar++)
	{
		QString artist = QString("artist %1").arg(ar);

		for(int al=0; al<albums; al++)
		{
			AlbumId album_id = AlbumId((ar*albums) + al);
			QString album = QString("album %1").arg(al);
			int year = 2000 + ar;

			for(int t=0; t<tracks; t++)
			{
				MetaData md;
				md.set_id(TrackID(track_id++));
				md.set_title(QString("title %1").arg(t));
				md.set_album(album);
				md.set_album_id(album_id);
				md.set_artist(artist);
				md.set_artist_id(ArtistId(ar));
				md.set_genres(genres[al]);
				md.set_track_number(TrackNum(t));
				md.set_rating(Rating::One);
				md.set_year(Year(year));
				md.set_library_id(0);
				QString dir = QString("%1/%2/%3 by %4")
						.arg(temp_path())
						.arg(md.year())
						.arg(md.album())
						.arg(md.artist());

				QString path =
					QString("%1/%2. %3.mp3")
						.arg(dir)
						.arg(md.track_number())
						.arg(md.title());

				md.set_filepath(path);

				if(!Util::File::exists(dir))
				{
					Util::File::create_directories(dir);
				}

				if(!Util::File::exists(path))
				{
					QString source = ":/test/mp3test.mp3";
					QString target = path;

					QFile f(source);
					f.copy(path);

					QFile f_new(path);
					f_new.setPermissions(
						QFile::Permission::ReadUser | QFile::Permission::ReadGroup | QFile::Permission::ReadOwner | QFile::Permission::ReadOther |
						QFile::Permission::WriteUser | QFile::Permission::WriteGroup | QFile::Permission::WriteOwner | QFile::Permission::WriteOther
					);

					Tagging::Utils::setMetaDataOfFile(md);
				}

				v_md << md;
			}
		}
	}

	return v_md;
}

void EditorTest::test_init()
{
	MetaDataList tracks = create_metadata(5, 3, 10);
	Editor* editor = new Editor(tracks);

	QVERIFY(editor->count() == (5*3*10));
	QVERIFY(editor->has_changes() == false);

	// 15 albums here
	QVERIFY(editor->can_load_entire_album() == false);

	for(int i=0; i<int(tracks.size()); i++)
	{
		const MetaData& md = tracks[i];
		QVERIFY(QFile::exists(md.filepath()));

		QVERIFY(editor->has_cover_replacement(i) == false);

		if(i < 10)
		{
			QVERIFY(editor->is_cover_supported(i) == true);
		}
	}

	QVERIFY(editor->failed_files().isEmpty());

	editor->deleteLater();
}

void EditorTest::test_rating()
{
	MetaDataList tracks = create_metadata(1, 1, 6);
	Editor* editor = new Editor(tracks);

	QVERIFY(editor->count() == (1*1*6));
	QVERIFY(editor->has_changes() == false);
	QVERIFY(editor->can_load_entire_album() == true);

	tracks[0].set_rating(Rating::Zero);
	tracks[1].set_rating(Rating::One);
	tracks[2].set_rating(Rating::Two);
	tracks[3].set_rating(Rating::Three);
	tracks[4].set_rating(Rating::Four);
	tracks[5].set_rating(Rating::Five);

	for(int i=0; i<6; i++)
	{
		editor->update_track(i, tracks[i]);
	}

	QVERIFY(editor->has_changes() == true);

	const MetaDataList updated_tracks = editor->metadata();
	for(int r=0; r<6; r++)
	{
		MetaData md = editor->metadata(r);
		QVERIFY(md.rating() == Rating(r));
	}
}

void EditorTest::test_one_album()
{
	MetaDataList tracks = create_metadata(1, 1, 10);
	Editor* editor = new Editor();
	editor->set_metadata(tracks);

	QVERIFY(editor->count() == 10);
	QVERIFY(editor->has_changes() == false);
	QVERIFY(editor->can_load_entire_album() == true);

	editor->deleteLater();
}


void EditorTest::test_changed_metadata()
{
	MetaDataList tracks = create_metadata(1, 1, 2);
	Editor* editor = new Editor();
	editor->set_metadata(tracks);

	QVERIFY(editor->count() == 2);
	QVERIFY(editor->has_changes() == false);
	QVERIFY(editor->can_load_entire_album() == true);

	tracks = create_metadata(2, 2, 5);
	editor->set_metadata(tracks);

	QVERIFY(editor->count() == 2*2*5);
	QVERIFY(editor->has_changes() == false);
	QVERIFY(editor->can_load_entire_album() == false);

	editor->deleteLater();
}

void EditorTest::test_edit()
{
	MetaDataList tracks = create_metadata(1, 1, 10);
	Editor* editor = new Editor();
	editor->set_metadata(tracks);

	QVERIFY(editor->count() == 10);
	QVERIFY(editor->has_changes() == false);
	QVERIFY(editor->can_load_entire_album() == true);

	for(int i=0; i<int(tracks.size()); i++)
	{
		MetaData md = tracks[i];

		if(i % 3 == 0) // 0, 3, 6, 9
		{
			md.set_title( QString("other %1").arg(i) );
			editor->update_track(i, md);
		}
	}

	QVERIFY(editor->has_changes() == true);
	MetaDataList editor_tracks = editor->metadata();

	for(int i=0; i<int(tracks.size()); i++)
	{
		MetaData md_org = tracks[i];
		MetaData md_changed = editor->metadata(i);

		if(i % 3 == 0)
		{
			QVERIFY(md_org.is_equal_deep(md_changed) == false);
		}

		else
		{
			QVERIFY(md_org.is_equal_deep(md_changed) == true);
		}

		QVERIFY(md_changed.is_equal_deep(editor_tracks[i]) == true);
	}

	editor->deleteLater();
}

void EditorTest::test_cover()
{
	MetaDataList tracks;

	auto* db = DB::Connector::instance();
	db->library_db(0,0)->getAllTracks(tracks);
	db->close_db();

	QVERIFY(tracks.size() == 2*2*10);

	Editor* editor = new Editor();
	editor->set_metadata(tracks);
	QVERIFY(editor->count() == tracks.count());
	QVERIFY(editor->has_changes() == false);

	QString name(":/test/logo.png");
	QPixmap cover(name);
	QVERIFY(cover.isNull() == false);

	for(int i=0; i<tracks.count(); i++)
	{
		QVERIFY(editor->has_cover_replacement(i) == false);
	}

	for(int i=0; i<10; i++)
	{
		editor->update_cover(i, cover);
	}

	QVERIFY(editor->has_changes() == false);

	for(int i=0; i<tracks.count(); i++)
	{
		QVERIFY(editor->has_cover_replacement(i) == (i < 10));
	}

	editor->deleteLater();
}


void EditorTest::test_commit()
{
	MetaDataList tracks;

	auto* db = DB::Connector::instance();
	db->library_db(0,0)->getAllTracks(tracks);
	db->close_db();

	QVERIFY(tracks.size() == 2*2*10);

	Editor* editor = new Editor();
	editor->set_metadata(tracks);

	auto* mdcn = Tagging::ChangeNotifier::instance();
	QSignalSpy spy(mdcn, &Tagging::ChangeNotifier::sig_metadata_changed);

	QVERIFY(editor->count() == tracks.count());
	QVERIFY(editor->has_changes() == false);
	QVERIFY(editor->can_load_entire_album() == false);

	for(int i=0; i<int(tracks.size()); i++)
	{
		if(i % 10 == 0) // 0, 10, 20, 30
		{
			MetaData md = tracks[i];
			md.set_title( QString("other %1").arg(i) );
			editor->update_track(i, md);
		}
	}

	auto* t = new QThread();
	t->setObjectName("EditorWorkingThreadForTest");
	editor->moveToThread(t);

	connect(t, &QThread::started, editor, &Editor::commit);
	connect(editor, &Editor::sig_finished, t, &QThread::quit);
	connect(t, &QThread::finished, t, &QObject::deleteLater);
	connect(t, &QThread::finished, editor, &QObject::deleteLater);

	t->start();

	spy.wait();

	QCOMPARE(spy.count(), 1);

	MetaDataList old_md = mdcn->changed_metadata().first;
	MetaDataList new_md = mdcn->changed_metadata().second;

	QVERIFY(old_md.size() == 4);
	QVERIFY(new_md.size() == 4);

	int cur_idx = 0;
	for(int i=0; i<int(tracks.size()); i++)
	{
		if(i % 10 == 0) // 0, 10, 20, 30
		{
			QVERIFY(old_md[cur_idx].is_equal_deep(tracks[i]));
			QVERIFY(new_md[cur_idx].filepath() == tracks[i].filepath());
			QVERIFY(new_md[cur_idx].title().startsWith("other"));

			cur_idx++;
		}
	}
}

QTEST_MAIN(EditorTest)

#include "EditorTest.moc"