#include "GUI_ControlsBase.h"
#include "SearchSlider.h"

#include "Gui/Covers/CoverButton.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Gui/Utils/Widgets/ProgressBar.h"
#include "Gui/Utils/Widgets/RatingLabel.h"
#include "Gui/Utils/Widgets/FloatingLabel.h"

#include "Components/PlayManager/PlayManager.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Engine/EngineHandler.h"
#include "Components/Tagging/ChangeNotifier.h"

#include "Utils/Algorithm.h"
#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"

#include <QToolTip>
#include <QImage>
#include <QIcon>
#include <QDateTime>

#include <algorithm>
#include <cmath>

static void setIcon(QPushButton* btn, const QIcon& icon)
{
	int width = Gui::Util::textWidth(btn->fontMetrics(), "MMn");

	QSize sz(width, width);
	btn->setFixedSize(sz);

	sz.setWidth((width * 800) / 1000);
	sz.setHeight((width * 800) / 1000);

	btn->setIconSize(sz);
	btn->setIcon(icon);
}

static MetaData currentTrack()
{
	return PlayManager::instance()->currentTrack();
}

struct GUI_ControlsBase::Private
{
	Library::ContextMenu* contextMenu = nullptr;
};

GUI_ControlsBase::GUI_ControlsBase(QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>();
}

GUI_ControlsBase::~GUI_ControlsBase() = default;

void GUI_ControlsBase::init()
{
	QString version = GetSetting(Set::Player_Version);

	labSayonara()->setText(tr("Sayonara Player"));
	labVersion()->setText(version);
	labWrittenBy()->setText(tr("Written by %1").arg("Michael Lugmair (Lucio Carreras)"));
	labCopyright()->setText(tr("Copyright") + " 2011 - " + QString::number(QDateTime::currentDateTime().date().year()));
	btnRecord()->setVisible(false);

	PlayManager* pm = PlayManager::instance();

	volumeChanged(pm->volume());
	muteChanged(pm->isMuted());

	setupConnections();
	setupShortcuts();

	if(pm->playstate() != PlayState::FirstStartup)
	{
		playstateChanged(pm->playstate());

		if(pm->playstate() != PlayState::Stopped)
		{
			currentTrackChanged(pm->currentTrack());
			currentPositionChanged(pm->initialPositionMs());
		}
	}

	btnCover()->setAlternativeSearchEnabled(false);
	connect(btnCover(), &Gui::CoverButton::sigRejected, this, &GUI_ControlsBase::coverClickRejected);

	ListenSetting(Set::Engine_SR_Active, GUI_ControlsBase::streamRecorderActiveChanged);
	ListenSetting(Set::Engine_Pitch, GUI_ControlsBase::refreshCurrentTrack);
	ListenSettingNoCall(Set::Engine_SpeedActive, GUI_ControlsBase::refreshCurrentTrack);

	skinChanged();
}

Gui::RatingEditor* GUI_ControlsBase::labRating() const
{
	return nullptr;
}

QSize GUI_ControlsBase::buttonSize() const
{
	return btnCover()->size();
}

// new track
void GUI_ControlsBase::currentTrackChanged(const MetaData& md)
{
	labSayonara()->hide();
	labVersion()->hide();
	labWrittenBy()->hide();
	labCopyright()->hide();

	labTitle()->show();
	labArtist()->show();
	labAlbum()->show();
	widgetDetails()->show();

	refreshCurrentPosition(0);
	refreshLabels(md);

	setCoverLocation(md);
	setRadioMode(md.radioMode());

	sliProgress()->setEnabled((md.durationMs() / 1000) > 0);
}

void GUI_ControlsBase::playstateChanged(PlayState state)
{
	switch(state)
	{
		case PlayState::Playing:
			played();
			break;
		case PlayState::Paused:
			paused();
			break;
		case PlayState::Stopped:
			stopped();
			break;
		default:
			break;
	}

	checkRecordButtonVisible();
}

QIcon GUI_ControlsBase::icon(Gui::Icons::IconName name)
{
	using namespace Gui;

	Icons::changeTheme();
	Icons::IconMode mode = Icons::Automatic;

	if(Style::isDark())
	{
		mode = Icons::ForceSayonaraIcon;
	}

	return Icons::icon(name, mode);
}

void GUI_ControlsBase::played()
{
	labCurrentTime()->setVisible(true);
	btnPlay()->setIcon(icon(Gui::Icons::Pause));
}

void GUI_ControlsBase::paused()
{
	labCurrentTime()->setVisible(true);
	btnPlay()->setIcon(icon(Gui::Icons::Play));
}

void GUI_ControlsBase::stopped()
{
	setWindowTitle("Sayonara");

	btnPlay()->setIcon(icon(Gui::Icons::Play));
	sliProgress()->set_buffering(-1);

	labTitle()->hide();
	labArtist()->hide();
	labAlbum()->hide();
	widgetDetails()->hide();

	labCurrentTime()->setText("00:00");
	labCurrentTime()->hide();
	labMaxTime()->clear();
	labMaxTime()->setVisible(false);

	labSayonara()->show();
	labWrittenBy()->show();
	labVersion()->show();
	labCopyright()->show();

	sliProgress()->setValue(0);
	sliProgress()->setEnabled(false);

	setStandardCover();

	if(labRating())
	{
		labRating()->hide();
	}
}

void GUI_ControlsBase::recordChanged(bool b)
{
	btnRecord()->setChecked(b);
}

void GUI_ControlsBase::buffering(int progress)
{
	// buffering
	if(progress > 0 && progress < 100)
	{
		sliProgress()->set_buffering(progress);

		labCurrentTime()->setText(QString("%1%").arg(progress));
		labMaxTime()->setVisible(false);
	}

		//buffering stopped
	else if(progress == 0)
	{
		sliProgress()->set_buffering(-1);
		labMaxTime()->setVisible(false);
	}

		// no buffering
	else
	{
		sliProgress()->set_buffering(-1);
		labMaxTime()->setVisible(currentTrack().durationMs() > 0);
	}
}

void GUI_ControlsBase::progressMoved(int val)
{
	val = std::max(val, 0);

	refreshCurrentPosition(val);

	double percent = (val * 1.0) / sliProgress()->maximum();
	PlayManager::instance()->seekRelative(percent);
}

void GUI_ControlsBase::currentPositionChanged(MilliSeconds pos_ms)
{
	spLog(Log::Crazy, this) << "Current position: " << pos_ms;

	MilliSeconds duration = PlayManager::instance()->durationMs();
	int max = sliProgress()->maximum();
	int new_val = 0;
	double percent = (pos_ms * 1.0) / duration;

	if(duration > 0)
	{
		new_val = int(max * percent);
	}

	else if(pos_ms > duration)
	{
		new_val = 0;
	}

	else
	{
		return;
	}

	if(!sliProgress()->is_busy())
	{
		QString cur_pos_string = Util::msToString(pos_ms, "$M:$S");
		labCurrentTime()->setText(cur_pos_string);
		sliProgress()->setValue(new_val);
	}
}

void GUI_ControlsBase::refreshCurrentPosition(int val)
{
	MilliSeconds duration = PlayManager::instance()->durationMs();
	int max = sliProgress()->maximum();

	val = std::max(val, 0);
	val = std::min(max, val);

	double percent = (val * 1.0) / max;

	MilliSeconds cur_pos_ms = MilliSeconds(percent * duration);
	QString cur_pos_string = Util::msToString(cur_pos_ms, "$M:$S");

	labCurrentTime()->setText(cur_pos_string);
}

void GUI_ControlsBase::setTotalTimeLabel(MilliSeconds total_time)
{
	QString length_str;
	if(total_time > 0)
	{
		length_str = Util::msToString(total_time, "$M:$S");
		labMaxTime()->setText(length_str);
	}

	labMaxTime()->setVisible(total_time > 0);
	sliProgress()->setEnabled(total_time > 0);
}

void GUI_ControlsBase::progressHovered(int val)
{
	MilliSeconds duration = PlayManager::instance()->durationMs();
	int max = sliProgress()->maximum();

	val = std::max(val, 0);
	val = std::min(max, val);

	double percent = (val * 1.0) / max;

	MilliSeconds cur_pos_ms = MilliSeconds(percent * duration);
	QString cur_pos_string = Util::msToString(cur_pos_ms, "$M:$S");

	QToolTip::showText(QCursor::pos(), cur_pos_string);
}

void GUI_ControlsBase::volumeChanged(int val)
{
	setupVolumeButton(val);
	sliVolume()->setValue(val);
}

void GUI_ControlsBase::setupVolumeButton(int percent)
{
	using namespace Gui;

	QIcon icon;

	if(percent <= 1 || PlayManager::instance()->isMuted())
	{
		icon = Icons::icon(Icons::VolMute);
	}

	else if(percent < 40)
	{
		icon = Icons::icon(Icons::Vol1);
	}

	else if(percent < 80)
	{
		icon = Icons::icon(Icons::Vol2);
	}

	else
	{
		icon = Icons::icon(Icons::Vol3);
	}

	setIcon(btnMute(), icon);
}

void GUI_ControlsBase::increaseVolume()
{
	PlayManager::instance()->volumeUp();
}

void GUI_ControlsBase::decreaseVolume()
{
	PlayManager::instance()->volumeDown();
}

void GUI_ControlsBase::changeVolumeByDelta(int val)
{
	if(val > 0)
	{
		increaseVolume();
	}

	else
	{
		decreaseVolume();
	}
}

void GUI_ControlsBase::muteChanged(bool muted)
{
	int val = PlayManager::instance()->volume();
	if(muted)
	{
		val = 0;
	}

	sliVolume()->setValue(val);
	setupVolumeButton(val);

	sliVolume()->setDisabled(muted);
}

// public slot:
// id3 tags have changed
void GUI_ControlsBase::metadataChanged()
{
	QList<MetaDataPair> changedTracks = Tagging::ChangeNotifier::instance()->changedMetadata();

	MetaData currentTrack = PlayManager::instance()->currentTrack();
	auto it = Util::Algorithm::find(changedTracks, [&currentTrack](const MetaDataPair& trackPair) {
		const MetaData& oldTrack = trackPair.first;
		return (oldTrack.filepath() == currentTrack.filepath());
	});

	if(it == changedTracks.end())
	{
		return;
	}

	const MetaData& newTrack = it->second;
	refreshLabels(newTrack);
	setCoverLocation(newTrack);

	setWindowTitle(QString("Sayonara - %1").arg(newTrack.title()));
}

void GUI_ControlsBase::refreshCurrentTrack()
{
	refreshLabels(currentTrack());
}

static void set_floating_text(QLabel* label, const QString& text)
{
	Gui::FloatingLabel* floating_label = dynamic_cast<Gui::FloatingLabel*>(label);
	if(floating_label)
	{
		floating_label->setFloatingText(text);
	}

	else
	{
		label->setText(text);
	}
}

void GUI_ControlsBase::refreshLabels(const MetaData& md)
{
	// title, artist
	set_floating_text(labTitle(), md.title());
	set_floating_text(labArtist(), md.artist());

	{ //album
		QString sYear = QString::number(md.year());
		QString album_name = md.album();

		labAlbum()->setToolTip("");
		if(md.year() > 1000 && (!album_name.contains(sYear)))
		{
			album_name += QString(" (%1)").arg(md.year());
		}

		else if(md.radioMode() == RadioMode::Station)
		{
			labAlbum()->setToolTip(md.filepath());
		}

		set_floating_text(labAlbum(), album_name);
	}

	{ // bitrate
		QString sBitrate;
		if(md.bitrate() / 1000 > 0)
		{
			sBitrate = QString("%1 kBit/s")
				.arg(std::nearbyint(md.bitrate() / 1000.0));

			labBitrate()->setText(sBitrate);
		}

		labBitrate()->setVisible(!sBitrate.isEmpty());
	}

	{ // filesize
		QString sFilesize;
		if(md.filesize() > 0)
		{
			sFilesize = QString::number(static_cast<double>(md.filesize() / 1024) / 1024.0, 'f', 2) + " MB";
			labFilesize()->setText(sFilesize);
		}

		labFilesize()->setVisible(!sFilesize.isEmpty());
	}

	{ // rating
		if(labRating())
		{
			labRating()->setVisible(md.radioMode() == RadioMode::Off);
			labRating()->setRating(md.rating());
		}
	}

	setTotalTimeLabel(md.durationMs());
}

void GUI_ControlsBase::skinChanged()
{
	btnPlay()->setObjectName("ControlPlayButton");
	btnRecord()->setObjectName("ControlRecButton");
	btnNext()->setObjectName("ControlFwdButton");
	btnPrevious()->setObjectName("ControlBwdButton");
	btnStop()->setObjectName("ControlStopButton");
	btnMute()->setObjectName("ControlMuteButton");

	Gui::Widget::skinChanged();

	using namespace Gui;

	setIcon(btnNext(), icon(Icons::Forward));
	setIcon(btnPrevious(), icon(Icons::Backward));

	if(PlayManager::instance()->playstate() == PlayState::Playing)
	{
		setIcon(btnPlay(), icon(Icons::Pause));
	}

	else
	{
		setIcon(btnPlay(), icon(Icons::Play));
	}

	setIcon(btnStop(), icon(Icons::Stop));
	setIcon(btnRecord(), icon(Icons::Record));

	setupVolumeButton(sliVolume()->value());
}

void GUI_ControlsBase::streamRecorderActiveChanged()
{
	checkRecordButtonVisible();
	btnRecord()->setChecked(false);
}

void GUI_ControlsBase::checkRecordButtonVisible()
{
	PlayManager* pm = PlayManager::instance();

	bool recording_enabled =
		(
			GetSetting(SetNoDB::MP3enc_found) &&    // Lame Available
			GetSetting(Set::Engine_SR_Active) &&    // Streamrecorder active
			(currentTrack().radioMode() != RadioMode::Off) &&        // Radio on
			(pm->playstate() == PlayState::Playing)    // Is Playing
		);

	btnPlay()->setVisible(!recording_enabled);
	btnRecord()->setVisible(recording_enabled);

	if(!recording_enabled)
	{
		btnRecord()->setChecked(false);
	}
}

void GUI_ControlsBase::setCoverLocation(const MetaData& md)
{
	Cover::Location cl = Cover::Location::coverLocation(md, false);

	btnCover()->setCoverLocation(cl);
}

void GUI_ControlsBase::setStandardCover()
{
	Cover::Location cl = Cover::Location::invalidLocation();
	btnCover()->setCoverLocation(cl);
}

void GUI_ControlsBase::coverChanged(const QByteArray& data, const QString& mimedata)
{
	btnCover()->setCoverData(data, mimedata);
}

void GUI_ControlsBase::coverClickRejected()
{
	showInfo();
}

void GUI_ControlsBase::setupConnections()
{
	PlayManager* pm = PlayManager::instance();

	connect(btnPlay(), &QPushButton::clicked, pm, &PlayManager::playPause);
	connect(btnNext(), &QPushButton::clicked, pm, &PlayManager::next);
	connect(btnPrevious(), &QPushButton::clicked, pm, &PlayManager::previous);
	connect(btnStop(), &QPushButton::clicked, pm, &PlayManager::stop);
	connect(btnMute(), &QPushButton::clicked, pm, &PlayManager::toggleMute);
	connect(btnRecord(), &QPushButton::clicked, pm, &PlayManager::record);

	connect(sliVolume(), &Gui::SearchSlider::sig_slider_moved, pm, &PlayManager::setVolume);
	connect(sliProgress(), &Gui::SearchSlider::sig_slider_moved, this, &GUI_ControlsBase::progressMoved);
	connect(sliProgress(), &Gui::SearchSlider::sigSliderHovered, this, &GUI_ControlsBase::progressHovered);

	connect(pm, &PlayManager::sigPlaystateChanged, this, &GUI_ControlsBase::playstateChanged);
	connect(pm, &PlayManager::sigCurrentTrackChanged, this, &GUI_ControlsBase::currentTrackChanged);
	connect(pm, &PlayManager::sigCurrentMetadataChanged, this, &GUI_ControlsBase::refreshCurrentTrack);
	connect(pm, &PlayManager::sigDurationChangedMs, this, &GUI_ControlsBase::refreshCurrentTrack);
	connect(pm, &PlayManager::sigBitrateChanged, this, &GUI_ControlsBase::refreshCurrentTrack);
	connect(pm, &PlayManager::sigPositionChangedMs, this, &GUI_ControlsBase::currentPositionChanged);
	connect(pm, &PlayManager::sigBuffering, this, &GUI_ControlsBase::buffering);
	connect(pm, &PlayManager::sigVolumeChanged, this, &GUI_ControlsBase::volumeChanged);
	connect(pm, &PlayManager::sigMuteChanged, this, &GUI_ControlsBase::muteChanged);
	connect(pm, &PlayManager::sigRecording, this, &GUI_ControlsBase::recordChanged);

	Engine::Handler* engine = Engine::Handler::instance();
	connect(engine, &Engine::Handler::sigCoverDataAvailable, this, &GUI_ControlsBase::coverChanged);

	Tagging::ChangeNotifier* mdcn = Tagging::ChangeNotifier::instance();
	connect(mdcn, &Tagging::ChangeNotifier::sigMetadataChanged, this, &GUI_ControlsBase::metadataChanged);
}

void GUI_ControlsBase::setupShortcuts()
{
	ShortcutHandler* sch = ShortcutHandler::instance();
	PlayManager* playManager = PlayManager::instance();

	sch->shortcut(ShortcutIdentifier::PlayPause).connect(this, playManager, SLOT(playPause()));
	sch->shortcut(ShortcutIdentifier::Stop).connect(this, playManager, SLOT(stop()));
	sch->shortcut(ShortcutIdentifier::Next).connect(this, playManager, SLOT(next()));
	sch->shortcut(ShortcutIdentifier::Prev).connect(this, playManager, SLOT(previous()));
	sch->shortcut(ShortcutIdentifier::VolDown).connect(this, playManager, SLOT(volumeDown()));
	sch->shortcut(ShortcutIdentifier::VolUp).connect(this, playManager, SLOT(volumeUp()));
	sch->shortcut(ShortcutIdentifier::SeekFwd).connect(this, [=]() {
		playManager->seekRelativeMs(2000);
	});

	sch->shortcut(ShortcutIdentifier::SeekBwd).connect(this, [=]() {
		playManager->seekRelativeMs(-2000);
	});

	sch->shortcut(ShortcutIdentifier::SeekFwdFast).connect(this, [=]() {
		MilliSeconds ms = playManager->durationMs() / 20;
		playManager->seekRelativeMs(ms);
	});

	sch->shortcut(ShortcutIdentifier::SeekBwdFast).connect(this, [=]() {
		MilliSeconds ms = playManager->durationMs() / 20;
		playManager->seekRelativeMs(-ms);
	});
}

void GUI_ControlsBase::setRadioMode(RadioMode radio)
{
	checkRecordButtonVisible();

	if(radio != RadioMode::Off)
	{
		buffering(0);
	}
}

MD::Interpretation GUI_ControlsBase::metadataInterpretation() const
{
	return MD::Interpretation::Tracks;
}

MetaDataList GUI_ControlsBase::infoDialogData() const
{
	PlayState ps = PlayManager::instance()->playstate();
	if(ps == PlayState::Stopped)
	{
		return MetaDataList();
	}

	return MetaDataList(currentTrack());
}

void GUI_ControlsBase::resizeEvent(QResizeEvent* e)
{
	Widget::resizeEvent(e);
	refreshCurrentTrack();
}

void GUI_ControlsBase::showEvent(QShowEvent* e)
{
	Widget::showEvent(e);
	refreshCurrentTrack();
}

void GUI_ControlsBase::contextMenuEvent(QContextMenuEvent* e)
{
	using Library::ContextMenu;

	if(!m->contextMenu)
	{
		m->contextMenu = new ContextMenu(this);
		m->contextMenu->showActions
			(
				(ContextMenu::EntryInfo |
				 ContextMenu::EntryLyrics |
				 ContextMenu::EntryEdit)
			);

		connect(m->contextMenu, &ContextMenu::sigEditClicked, this, [=]() {
			showEdit();
		});

		connect(m->contextMenu, &ContextMenu::sigInfoClicked, this, [=]() {
			showInfo();
		});

		connect(m->contextMenu, &ContextMenu::sigLyricsClicked, this, [=]() {
			showLyrics();
		});

		m->contextMenu->addPreferenceAction(new Gui::PlayerPreferencesAction(m->contextMenu));
		m->contextMenu->addPreferenceAction(new Gui::CoverPreferenceAction(m->contextMenu));
	}

	m->contextMenu->exec(e->globalPos());
}
