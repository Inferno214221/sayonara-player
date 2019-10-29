#include <QTest>
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
	public QObject,
	public DB::ConnectorProvider
{
	Q_OBJECT

	QString m_tmp_path;

public:
	EditorTest(QObject* parent=nullptr);
	~EditorTest();
	MetaDataList create_metadata(int artists, int albums, int tracks);

	DB::Connector* get_connector() const override;

private slots:
	void test_init();
	void test_rating();
	void test_one_album();
	void test_changed_metadata();
	void test_edit();
	void test_commit();
};

EditorTest::EditorTest(QObject* parent) : QObject(parent)
{
	this->setObjectName("EditorTest");

	m_tmp_path = ::Util::temp_path("EditorTest");

	Util::File::delete_files( {m_tmp_path} );
	Util::File::create_directories(m_tmp_path);

	auto* db = get_connector();
	db->register_library_db(0);

	auto* lib_db = db->library_db(0, DbId(0));
	lib_db->store_metadata(create_metadata(2, 2, 10));

	db->close_db();
}

EditorTest::~EditorTest()
{
	Util::File::delete_files( {m_tmp_path} );
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
				md.id = TrackID(track_id++);
				md.set_title(QString("title %1").arg(t));
				md.set_album(album);
				md.album_id = album_id;
				md.set_artist(artist);
				md.artist_id = ArtistId(ar);
				md.set_genres(genres[al]);
				md.track_num = uint16_t(t);
				md.rating = Rating::One;
				md.year = uint16_t(year);
				md.library_id = 0;
				QString dir = QString("%1/%2/%3 by %4")
						.arg(m_tmp_path)
						.arg(md.year)
						.arg(md.album())
						.arg(md.artist());

				QString path =
					QString("%1/%2. %3.mp3")
						.arg(dir)
						.arg(md.track_num)
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

DB::Connector* EditorTest::get_connector() const
{
	 return DB::Connector::instance_custom("", m_tmp_path, "player.db");
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
		QVERIFY(editor->has_cover_replacement(i) == false);

		if(i < 10)
		{
			// file does not exist
			QVERIFY(editor->is_cover_supported(i) == false);
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

	tracks[0].rating = Rating::Zero;
	tracks[1].rating = Rating::One;
	tracks[2].rating = Rating::Two;
	tracks[3].rating = Rating::Three;
	tracks[4].rating = Rating::Four;
	tracks[5].rating = Rating::Five;

	for(int i=0; i<6; i++)
	{
		editor->update_track(i, tracks[i]);
	}

	QVERIFY(editor->has_changes() == true);

	const MetaDataList updated_tracks = editor->metadata();
	for(int r=0; r<6; r++)
	{
		MetaData md = editor->metadata(r);
		QVERIFY(md.rating == Rating(r));
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


void EditorTest::test_commit()
{
	MetaDataList tracks;

	auto* db = get_connector();
	db->library_db(0,0)->getAllTracks(tracks);
	db->close_db();

	QVERIFY(tracks.size() == 2*2*10);

	Editor* editor = new Editor();
	editor->set_metadata(tracks);
	editor->register_db_connector_provider(this);

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

	QTest::qWait(3000);
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

QTEST_GUILESS_MAIN(EditorTest)

#include "EditorTest.moc"
