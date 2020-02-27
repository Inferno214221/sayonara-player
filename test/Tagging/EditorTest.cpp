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
	DB::LibraryDatabase* mLibraryDatabase=nullptr;

public:
	EditorTest();
	~EditorTest()=default;

	MetaDataList createMetadata(int artists, int albums, int tracks);

private slots:
	void testInit();
	void testRating();
	void testOneAlbum();
	void testChangedMetadata();
	void testEdit();
	void testCover();
	void testCommit();
};

EditorTest::EditorTest() :
	Test::Base("EditorTest")
{
	auto* db = DB::Connector::instance();
	db->registerLibraryDatabase(0);

	mLibraryDatabase = db->libraryDatabase(0, 0);
	mLibraryDatabase->storeMetadata(createMetadata(2, 2, 10));
}

MetaDataList EditorTest::createMetadata(int artists, int albums, int trackCount)
{
	QStringList genres[]
	{
		{"Metal", "Folk", "Blues"},
		{"Pop"},
		{""}
	};

	TrackID trackId = 1;

	MetaDataList tracks;
	for(int ar=0; ar<artists; ar++)
	{
		QString artist = QString("artist %1").arg(ar);

		for(int al=0; al<albums; al++)
		{
			AlbumId albumId = AlbumId((ar*albums) + al);
			QString album = QString("album %1").arg(al);
			int year = 2000 + ar;

			for(int t=0; t<trackCount; t++)
			{
				MetaData md;
				md.setId(TrackID(trackId++));
				md.setTitle(QString("title %1").arg(t));
				md.setAlbum(album);
				md.setAlbumId(albumId);
				md.setArtist(artist);
				md.setArtistId(ArtistId(ar));
				md.setGenres(genres[al]);
				md.setTrackNumber(TrackNum(t));
				md.setRating(Rating::One);
				md.setYear(Year(year));
				md.setLibraryid(0);
				QString dir = QString("%1/%2/%3 by %4")
						.arg(tempPath())
						.arg(md.year())
						.arg(md.album())
						.arg(md.artist());

				QString path =
					QString("%1/%2. %3.mp3")
						.arg(dir)
						.arg(md.trackNumber())
						.arg(md.title());

				md.setFilepath(path);

				if(!Util::File::exists(dir))
				{
					Util::File::createDirectories(dir);
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

				tracks << md;
			}
		}
	}

	return tracks;
}

void EditorTest::testInit()
{
	MetaDataList tracks = createMetadata(5, 3, 10);
	Editor* editor = new Editor(tracks);

	QVERIFY(editor->count() == (5*3*10));
	QVERIFY(editor->hasChanges() == false);

	// 15 albums here
	QVERIFY(editor->canLoadEntireAlbum() == false);

	for(int i=0; i<int(tracks.size()); i++)
	{
		const MetaData& md = tracks[i];
		QVERIFY(QFile::exists(md.filepath()));

		QVERIFY(editor->hasCoverReplacement(i) == false);

		if(i < 10)
		{
			QVERIFY(editor->isCoverSupported(i) == true);
		}
	}

	QVERIFY(editor->failedFiles().isEmpty());

	editor->deleteLater();
}

void EditorTest::testRating()
{
	MetaDataList tracks = createMetadata(1, 1, 6);
	Editor* editor = new Editor(tracks);

	QVERIFY(editor->count() == (1*1*6));
	QVERIFY(editor->hasChanges() == false);
	QVERIFY(editor->canLoadEntireAlbum() == true);

	tracks[0].setRating(Rating::Zero);
	tracks[1].setRating(Rating::One);
	tracks[2].setRating(Rating::Two);
	tracks[3].setRating(Rating::Three);
	tracks[4].setRating(Rating::Four);
	tracks[5].setRating(Rating::Five);

	for(int i=0; i<6; i++)
	{
		editor->updateTrack(i, tracks[i]);
	}

	QVERIFY(editor->hasChanges() == true);

	const MetaDataList updatedTracks = editor->metadata();
	for(int r=0; r<6; r++)
	{
		MetaData md = editor->metadata(r);
		QVERIFY(md.rating() == Rating(r));
	}
}

void EditorTest::testOneAlbum()
{
	MetaDataList tracks = createMetadata(1, 1, 10);
	Editor* editor = new Editor();
	editor->setMetadata(tracks);

	QVERIFY(editor->count() == 10);
	QVERIFY(editor->hasChanges() == false);
	QVERIFY(editor->canLoadEntireAlbum() == true);

	editor->deleteLater();
}


void EditorTest::testChangedMetadata()
{
	MetaDataList tracks = createMetadata(1, 1, 2);
	Editor* editor = new Editor();
	editor->setMetadata(tracks);

	QVERIFY(editor->count() == 2);
	QVERIFY(editor->hasChanges() == false);
	QVERIFY(editor->canLoadEntireAlbum() == true);

	tracks = createMetadata(2, 2, 5);
	editor->setMetadata(tracks);

	QVERIFY(editor->count() == 2*2*5);
	QVERIFY(editor->hasChanges() == false);
	QVERIFY(editor->canLoadEntireAlbum() == false);

	editor->deleteLater();
}

void EditorTest::testEdit()
{
	MetaDataList tracks = createMetadata(1, 1, 10);
	Editor* editor = new Editor();
	editor->setMetadata(tracks);

	QVERIFY(editor->count() == 10);
	QVERIFY(editor->hasChanges() == false);
	QVERIFY(editor->canLoadEntireAlbum() == true);

	for(int i=0; i<int(tracks.size()); i++)
	{
		MetaData md = tracks[i];

		if(i % 3 == 0) // 0, 3, 6, 9
		{
			md.setTitle( QString("other %1").arg(i) );
			editor->updateTrack(i, md);
		}
	}

	QVERIFY(editor->hasChanges() == true);
	MetaDataList editor_tracks = editor->metadata();

	for(int i=0; i<int(tracks.size()); i++)
	{
		MetaData md_org = tracks[i];
		MetaData md_changed = editor->metadata(i);

		if(i % 3 == 0)
		{
			QVERIFY(md_org.isEqualDeep(md_changed) == false);
		}

		else
		{
			QVERIFY(md_org.isEqualDeep(md_changed) == true);
		}

		QVERIFY(md_changed.isEqualDeep(editor_tracks[i]) == true);
	}

	editor->deleteLater();
}

void EditorTest::testCover()
{
	MetaDataList tracks;

	mLibraryDatabase->getAllTracks(tracks);

	QVERIFY(tracks.size() == 2*2*10);

	Editor* editor = new Editor();
	editor->setMetadata(tracks);
	QVERIFY(editor->count() == tracks.count());
	QVERIFY(editor->hasChanges() == false);

	QString name(":/test/logo.png");
	QPixmap cover(name);
	QVERIFY(cover.isNull() == false);

	for(int i=0; i<tracks.count(); i++)
	{
		QVERIFY(editor->hasCoverReplacement(i) == false);
	}

	for(int i=0; i<10; i++)
	{
		editor->updateCover(i, cover);
	}

	QVERIFY(editor->hasChanges() == false);

	for(int i=0; i<tracks.count(); i++)
	{
		QVERIFY(editor->hasCoverReplacement(i) == (i < 10));
	}

	editor->deleteLater();
}


void EditorTest::testCommit()
{
	MetaDataList tracks;

	mLibraryDatabase->getAllTracks(tracks);

	QVERIFY(tracks.size() == 2*2*10);

	Editor* editor = new Editor();
	editor->setMetadata(tracks);

	auto* mdcn = Tagging::ChangeNotifier::instance();
	QSignalSpy spy(mdcn, &Tagging::ChangeNotifier::sigMetadataChanged);

	QVERIFY(editor->count() == tracks.count());
	QVERIFY(editor->hasChanges() == false);
	QVERIFY(editor->canLoadEntireAlbum() == false);

	for(int i=0; i<int(tracks.size()); i++)
	{
		if(i % 10 == 0) // 0, 10, 20, 30
		{
			MetaData md = tracks[i];
			md.setTitle( QString("other %1").arg(i) );
			editor->updateTrack(i, md);
		}
	}

	auto* t = new QThread();
	editor->moveToThread(t);

	connect(t, &QThread::started, editor, &Editor::commit);
	connect(editor, &Editor::sigFinished, t, &QThread::quit);
	connect(t, &QThread::finished, t, &QObject::deleteLater);
	connect(t, &QThread::finished, editor, &QObject::deleteLater);

	t->start();

	spy.wait();

	QCOMPARE(spy.count(), 1);

	auto changedMetaData = mdcn->changedMetadata();

	const MetaDataList& old_md = changedMetaData.first;
	const MetaDataList& new_md = changedMetaData.second;

	QVERIFY(old_md.size() == 4);
	QVERIFY(new_md.size() == 4);

	int cur_idx = 0;
	for(int i=0; i<int(tracks.size()); i++)
	{
		if(i % 10 == 0) // 0, 10, 20, 30
		{
			QVERIFY(old_md[cur_idx].isEqualDeep(tracks[i]));
			QVERIFY(new_md[cur_idx].filepath() == tracks[i].filepath());
			QVERIFY(new_md[cur_idx].title().startsWith("other"));

			cur_idx++;
		}
	}
}

QTEST_MAIN(EditorTest)

#include "EditorTest.moc"
