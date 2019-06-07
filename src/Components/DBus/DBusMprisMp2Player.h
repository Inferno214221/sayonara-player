#ifndef DBUSMPRISMP2PLAYER_H
#define DBUSMPRISMP2PLAYER_H

#include <QObject>
#include <QDBusObjectPath>
#include "Utils/Pimpl.h"
#include "Components/PlayManager/PlayState.h"
#include "DBusAdaptor.h"

class MetaData;

namespace DBusMPRIS
{

	class MediaPlayer2Player :
			public DBusAdaptor
	{
		Q_OBJECT
		PIMPL(MediaPlayer2Player)

		private:
			void init();

		public:
			explicit MediaPlayer2Player(QObject* parent=nullptr);
			~MediaPlayer2Player();

			Q_PROPERTY(QString		PlaybackStatus		READ	PlaybackStatus)
			QString					PlaybackStatus();


			Q_PROPERTY(QString		LoopStatus			READ	LoopStatus	WRITE	SetLoopStatus)
			QString					LoopStatus();
			void					SetLoopStatus(QString status);


			Q_PROPERTY(double		Rate				READ	Rate		WRITE	SetRate)
			double					Rate();
			void					SetRate(double rate);


			Q_PROPERTY(bool			Shuffle				READ	Shuffle		WRITE	SetShuffle)
			bool					Shuffle();
			void					SetShuffle(bool shuffle);


			Q_PROPERTY(QVariantMap	Metadata			READ	Metadata)
			QVariantMap				Metadata();


			Q_PROPERTY(double		Volume				READ	Volume		WRITE	SetVolume)
			double					Volume();
			void					SetVolume(double volume);


			Q_PROPERTY(qlonglong	Position			READ	Position)
			qlonglong				Position();
			void					SetPosition(const QDBusObjectPath& track_id, qlonglong position);


			Q_PROPERTY(double		MinimumRate			READ	MinimumRate)
			double					MinimumRate();


			Q_PROPERTY(double		MaximumRate			READ	MaximumRate)
			double					MaximumRate();


			Q_PROPERTY(bool			CanGoNext			READ	CanGoNext)
			bool					CanGoNext();


			Q_PROPERTY(bool			CanGoPrevious		READ	CanGoPrevious)
			bool					CanGoPrevious();


			Q_PROPERTY(bool			CanPlay				READ	CanPlay)
			bool					CanPlay();


			Q_PROPERTY(bool			CanPause			READ	CanPause)
			bool					CanPause();


			Q_PROPERTY(bool			CanSeek				READ	CanSeek)
			bool					CanSeek();


			Q_PROPERTY(bool			CanControl			READ	CanControl)
			bool					CanControl();


			void					Next();
			void					Previous();
			void					Pause();
			void					PlayPause();
			void					Stop();
			void					Play();
			void					Seek(qlonglong offset);

			void					OpenUri(const QString& uri);


		public slots:
			void					position_changed(MilliSeconds pos_ms);
			void					volume_changed(int volume);
			void					track_idx_changed(int idx);
			void					track_changed(const MetaData& md);
			void					playstate_changed(PlayState state);


		signals:
			void Seeked(qlonglong position);
	};

}
#endif // DBUSMPRISMP2PLAYER_H
