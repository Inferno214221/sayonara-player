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
#include "Gui/Utils/RatingLabel.h"
#include "Gui/Utils/Widgets/FloatingLabel.h"

#include "Components/PlayManager/PlayManager.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Engine/EngineHandler.h"
#include "Components/Tagging/ChangeNotifier.h"

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

static void set_icon(QPushButton* btn, QIcon icon)
{
	int w = (btn->fontMetrics().width("m") * 280) / 100;

	QSize sz(w, w);
	btn->setFixedSize(sz);

	sz.setWidth((w * 800) / 1000);
	sz.setHeight((w * 800) / 1000);

	btn->setIconSize(sz);
	btn->setIcon(icon);
}

static MetaData current_track()
{
	return PlayManager::instance()->current_track();
}

struct GUI_ControlsBase::Private
{
	Gui::LibraryContextMenu* context_menu=nullptr;
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

	lab_sayonara()->setText(tr("Sayonara Player"));
	lab_version()->setText( version );
	lab_writtenby()->setText(tr("Written by") + " Lucio Carreras");
	lab_copyright()->setText(tr("Copyright") + " 2011 - " + QString::number(QDateTime::currentDateTime().date().year()));
	btn_rec()->setVisible(false);

	PlayManager* pm = PlayManager::instance();

	volume_changed(pm->volume());
	mute_changed(pm->is_muted());

	setup_connections();
	setup_shortcuts();

	if(pm->playstate() != PlayState::FirstStartup)
	{
		playstate_changed(pm->playstate());

		if(pm->playstate() != PlayState::Stopped)
		{
			track_changed(pm->current_track());
			cur_pos_changed(pm->initial_position_ms());
		}
	}

	connect(btn_cover(), &Gui::CoverButton::sig_rejected, this, &GUI_ControlsBase::cover_click_rejected);

	ListenSetting(Set::Engine_SR_Active, GUI_ControlsBase::sr_active_changed);
	ListenSetting(Set::Engine_Pitch, GUI_ControlsBase::refresh_current_track);
	ListenSettingNoCall(Set::Engine_SpeedActive, GUI_ControlsBase::refresh_current_track);

	skin_changed();
}

Gui::RatingLabel* GUI_ControlsBase::lab_rating() const
{
	return nullptr;
}

QSize GUI_ControlsBase::button_size() const
{
	return btn_cover()->size();
}


// new track
void GUI_ControlsBase::track_changed(const MetaData& md)
{
	lab_sayonara()->hide();
	lab_version()->hide();
	lab_writtenby()->hide();
	lab_copyright()->hide();

	lab_title()->show();
	lab_artist()->show();
	lab_album()->show();
	widget_details()->show();

	refresh_current_position(0);
	refresh_labels(md);

	set_cover_location(md);
	set_radio_mode( md.radio_mode() );

	sli_progress()->setEnabled( (md.duration_ms / 1000) > 0 );
}


void GUI_ControlsBase::playstate_changed(PlayState state)
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

	check_record_button_visible();	
}


QIcon GUI_ControlsBase::icon(Gui::Icons::IconName name)
{
	using namespace Gui;

	Icons::change_theme();
	Icons::IconMode mode = Icons::Automatic;

	if(Style::is_dark()){
		mode = Icons::ForceSayonaraIcon;
	}

	return Icons::icon(name, mode);
}

void GUI_ControlsBase::played()
{
	lab_current_time()->setVisible(true);
	btn_play()->setIcon(icon(Gui::Icons::Pause));
}

void GUI_ControlsBase::paused()
{
	lab_current_time()->setVisible(true);
	btn_play()->setIcon(icon(Gui::Icons::Play));
}

void GUI_ControlsBase::stopped()
{
	setWindowTitle("Sayonara");

	btn_play()->setIcon(icon(Gui::Icons::Play));
	sli_progress()->set_buffering(-1);

	lab_title()->hide();
	lab_artist()->hide();
	lab_album()->hide();
	widget_details()->hide();

	lab_current_time()->setText("00:00");
	lab_current_time()->hide();
	lab_max_time()->clear();
	lab_max_time()->setVisible(false);

	lab_sayonara()->show();
	lab_writtenby()->show();
	lab_version()->show();
	lab_copyright()->show();

	sli_progress()->setValue(0);
	sli_progress()->setEnabled(false);

	set_standard_cover();

	if(lab_rating()){
		lab_rating()->hide();
	}
}

void GUI_ControlsBase::rec_changed(bool b)
{
	btn_rec()->setChecked(b);
}

void GUI_ControlsBase::buffering(int progress)
{
	// buffering
	if(progress > 0 && progress < 100)
	{
		sli_progress()->set_buffering(progress);

		lab_current_time()->setText(QString("%1%").arg(progress));
		lab_max_time()->setVisible(false);
	}

	//buffering stopped
	else if(progress == 0)
	{
		sli_progress()->set_buffering(-1);
		lab_max_time()->setVisible(false);
	}

	// no buffering
	else
	{
		sli_progress()->set_buffering(-1);
		lab_max_time()->setVisible(current_track().duration_ms > 0);
	}
}

void GUI_ControlsBase::progress_moved(int val)
{
	val = std::max(val, 0);

	refresh_current_position(val);

	double percent = (val * 1.0) / sli_progress()->maximum();
	PlayManager::instance()->seek_rel(percent);
}


void GUI_ControlsBase::cur_pos_changed(MilliSeconds pos_ms)
{
	sp_log(Log::Crazy, this) << "Current position: " << pos_ms;

	MilliSeconds duration = PlayManager::instance()->duration_ms();
	int max = sli_progress()->maximum();
	int new_val=0;
	double percent = (pos_ms * 1.0) / duration;

	if ( duration > 0 ) {
		new_val = int(max * percent);
	}

	else if(pos_ms > duration) {
		new_val = 0;
	}

	else{
		return;
	}

	if(!sli_progress()->is_busy())
	{
		QString cur_pos_string = Util::cvt_ms_to_string(pos_ms);
		lab_current_time()->setText(cur_pos_string);
		sli_progress()->setValue(new_val);
	}
}


void GUI_ControlsBase::refresh_current_position(int val)
{
	MilliSeconds duration = PlayManager::instance()->duration_ms();
	int max = sli_progress()->maximum();

	val = std::max(val, 0);
	val = std::min(max, val);

	double percent = (val * 1.0) / max;

	MilliSeconds cur_pos_ms = MilliSeconds(percent * duration);
	QString cur_pos_string = Util::cvt_ms_to_string(cur_pos_ms);

	lab_current_time()->setText(cur_pos_string);
}

void GUI_ControlsBase::set_total_time_label(MilliSeconds total_time)
{
	QString length_str;
	if(total_time > 0){
		length_str = Util::cvt_ms_to_string(total_time, true);
		lab_max_time()->setText(length_str);
	}

	lab_max_time()->setVisible(total_time > 0);
	sli_progress()->setEnabled(total_time > 0);
}

void GUI_ControlsBase::progress_hovered(int val)
{
	MilliSeconds duration = PlayManager::instance()->duration_ms();
	int max = sli_progress()->maximum();

	val = std::max(val, 0);
	val = std::min(max, val);

	double percent = (val * 1.0) / max;

	MilliSeconds cur_pos_ms = MilliSeconds(percent * duration);
	QString cur_pos_string = Util::cvt_ms_to_string(cur_pos_ms);

	QToolTip::showText( QCursor::pos(), cur_pos_string );
}


void GUI_ControlsBase::volume_changed(int val)
{
	setup_volume_button(val);
	sli_volume()->setValue(val);
}

void GUI_ControlsBase::setup_volume_button(int percent)
{
	using namespace Gui;

	QIcon icon;

	if (percent <= 1 || PlayManager::instance()->is_muted()) {
		icon = Icons::icon(Icons::VolMute);
	}

	else if (percent < 40) {
		icon = Icons::icon(Icons::Vol1);
	}

	else if (percent < 80) {
		icon = Icons::icon(Icons::Vol2);
	}

	else {
		icon = Icons::icon(Icons::Vol3);
	}

	set_icon(btn_mute(), icon);
}

void GUI_ControlsBase::increase_volume()
{
	PlayManager::instance()->volume_up();
}

void GUI_ControlsBase::decrease_volume()
{
	PlayManager::instance()->volume_down();
}

void GUI_ControlsBase::change_volume_by_tick(int val)
{
	if(val > 0) {
		increase_volume();
	}

	else {
		decrease_volume();
	}
}

void GUI_ControlsBase::mute_changed(bool muted)
{
	int val = PlayManager::instance()->volume();
	if(muted) {
		val = 0;
	}

	sli_volume()->setValue(val);
	setup_volume_button(val);

	sli_volume()->setDisabled(muted);
}


// public slot:
// id3 tags have changed
void GUI_ControlsBase::id3_tags_changed()
{
	auto changed_metadata = Tagging::ChangeNotifier::instance()->changed_metadata();

	IdxList idxs = changed_metadata.first.findTracks(current_track().filepath());
	if(idxs.empty()) {
		return;
	}

	MetaData md = changed_metadata.second[idxs.first()];

	refresh_labels(md);
	set_cover_location(md);

	setWindowTitle(
		QString("Sayonara - %1").arg(md.title())
	);
}

void GUI_ControlsBase::refresh_current_track()
{
	refresh_labels( current_track() );
}

static void set_floating_text(QLabel* label, const QString& text)
{
	Gui::FloatingLabel* floating_label = dynamic_cast<Gui::FloatingLabel*>(label);
	if(floating_label) {
		floating_label->setFloatingText(text);
	}

	else {
		label->setText(text);
	}
}

void GUI_ControlsBase::refresh_labels(const MetaData& md)
{
	// title, artist
	set_floating_text(lab_title(), md.title());
	set_floating_text(lab_artist(), md.artist());

	{ //album
		QString str_year = QString::number(md.year);
		QString album_name;

		lab_album()->setToolTip("");
		if(md.year > 1000 && !album_name.contains(str_year)) {
			album_name = md.album() + " (" + str_year + ")";
		}

		else if(md.radio_mode() == RadioMode::Station) {
			album_name = md.album();
			lab_album()->setToolTip(md.filepath());
		}

		set_floating_text(lab_album(), album_name);
	}

	{ // pitch/speed

		if( (GetSetting(Set::Engine_Pitch) != 440) &&
			GetSetting(Set::Engine_SpeedActive))
		{
			QString pitch_text = QString::number(GetSetting(Set::Engine_Pitch)) + "Hz";
			Q_UNUSED(pitch_text)
		}
	}

	{ // bitrate
		QString sBitrate;
		if(md.bitrate / 1000 > 0)
		{
			sBitrate = QString::number(std::nearbyint(md.bitrate / 1000.0)) + " kBit/s";
			lab_bitrate()->setText(sBitrate);
		}

		lab_bitrate()->setVisible(!sBitrate.isEmpty());
	}

	{ // filesize
		QString sFilesize;
		if(md.filesize > 0)
		{
			sFilesize = QString::number( static_cast<double>(md.filesize / 1024) / 1024.0, 'f', 2) + " MB";
			lab_filesize()->setText(sFilesize);
		}

		lab_filesize()->setVisible(!sFilesize.isEmpty());
	}

	{ // rating
		if(lab_rating())
		{
			lab_rating()->setVisible(md.radio_mode() == RadioMode::Off);
			lab_rating()->set_rating(md.rating);
		}
	}

	set_total_time_label(md.duration_ms);
}


void GUI_ControlsBase::skin_changed()
{
	Gui::Widget::skin_changed();

	using namespace Gui;

	set_icon(btn_fwd(), icon(Icons::Forward));
	set_icon(btn_bwd(), icon(Icons::Backward));

	if(PlayManager::instance()->playstate() == PlayState::Playing)
	{
		set_icon(btn_play(), icon(Icons::Pause));
	}

	else
	{
		set_icon(btn_play(), icon(Icons::Play));
	}

	set_icon(btn_stop(), icon(Icons::Stop));
	set_icon(btn_rec(), icon(Icons::Record));

	setup_volume_button(sli_volume()->value());
}


void GUI_ControlsBase::sr_active_changed()
{
	check_record_button_visible();
	btn_rec()->setChecked(false);
}


void GUI_ControlsBase::check_record_button_visible()
{
	PlayManager* pm = PlayManager::instance();

	bool recording_enabled =
	(
		GetSetting(SetNoDB::MP3enc_found) &&	// Lame Available
		GetSetting(Set::Engine_SR_Active) &&	// Streamrecorder active
		(current_track().radio_mode() != RadioMode::Off) &&		// Radio on
		(pm->playstate() == PlayState::Playing)	// Is Playing
	);

	btn_play()->setVisible(!recording_enabled);
	btn_rec()->setVisible(recording_enabled);

	if(!recording_enabled) {
		btn_rec()->setChecked(false);
	}
}


void GUI_ControlsBase::set_cover_location(const MetaData& md)
{
	auto cl = Cover::Location::cover_location(md);
	btn_cover()->set_cover_location(cl);
}

void GUI_ControlsBase::set_standard_cover()
{
	auto cl = Cover::Location::invalid_location();
	btn_cover()->set_cover_location(cl);
}

void GUI_ControlsBase::cover_changed(const QImage& img)
{
	btn_cover()->force_cover(img);
}

void GUI_ControlsBase::cover_click_rejected()
{
	show_cover_edit();
}

void GUI_ControlsBase::setup_connections()
{
	PlayManager* pm = PlayManager::instance();

	connect(btn_play(), &QPushButton::clicked, pm, &PlayManager::play_pause);
	connect(btn_fwd(),	&QPushButton::clicked, pm, &PlayManager::next);
	connect(btn_bwd(),	&QPushButton::clicked, pm, &PlayManager::previous);
	connect(btn_stop(), &QPushButton::clicked, pm, &PlayManager::stop);
	connect(btn_mute(), &QPushButton::clicked, pm, &PlayManager::toggle_mute);
	connect(btn_rec(), &QPushButton::clicked, pm, &PlayManager::record);

	connect(sli_volume(), &Gui::SearchSlider::sig_slider_moved, pm, &PlayManager::set_volume);
	connect(sli_progress(), &Gui::SearchSlider::sig_slider_moved, this, &GUI_ControlsBase::progress_moved);
	connect(sli_progress(), &Gui::SearchSlider::sig_slider_hovered, this, &GUI_ControlsBase::progress_hovered);

	connect(pm, &PlayManager::sig_playstate_changed, this, &GUI_ControlsBase::playstate_changed);
	connect(pm, &PlayManager::sig_track_changed, this, &GUI_ControlsBase::track_changed);
	connect(pm, &PlayManager::sig_track_metadata_changed, this, &GUI_ControlsBase::refresh_current_track);
	connect(pm, &PlayManager::sig_duration_changed, this, &GUI_ControlsBase::refresh_current_track);
	connect(pm, &PlayManager::sig_bitrate_changed,	this, &GUI_ControlsBase::refresh_current_track);
	connect(pm, &PlayManager::sig_position_changed_ms, this,	&GUI_ControlsBase::cur_pos_changed);
	connect(pm, &PlayManager::sig_buffer, this, &GUI_ControlsBase::buffering);
	connect(pm, &PlayManager::sig_volume_changed, this, &GUI_ControlsBase::volume_changed);
	connect(pm, &PlayManager::sig_mute_changed, this, &GUI_ControlsBase::mute_changed);
	connect(pm, &PlayManager::sig_record, this, &GUI_ControlsBase::rec_changed);

	Engine::Handler* engine = Engine::Handler::instance();
	connect(engine, &Engine::Handler::sig_cover_changed, this, &GUI_ControlsBase::cover_changed);

	Tagging::ChangeNotifier* mdcn = Tagging::ChangeNotifier::instance();
	connect(mdcn, &Tagging::ChangeNotifier::sig_metadata_changed, this, &GUI_ControlsBase::id3_tags_changed);
}


void GUI_ControlsBase::setup_shortcuts()
{
	ShortcutHandler* sch = ShortcutHandler::instance();
	PlayManager* play_manager = PlayManager::instance();

	sch->shortcut(ShortcutIdentifier::PlayPause).connect(this, play_manager, SLOT(play_pause()));
	sch->shortcut(ShortcutIdentifier::Stop).connect(this, play_manager, SLOT(stop()));
	sch->shortcut(ShortcutIdentifier::Next).connect(this, play_manager, SLOT(next()));
	sch->shortcut(ShortcutIdentifier::Prev).connect(this, play_manager, SLOT(previous()));
	sch->shortcut(ShortcutIdentifier::VolDown).connect(this, play_manager, SLOT(volume_down()));
	sch->shortcut(ShortcutIdentifier::VolUp).connect(this, play_manager, SLOT(volume_up()));
	sch->shortcut(ShortcutIdentifier::SeekFwd).connect(this, [=]() {
		play_manager->seek_rel_ms(2000);
	});

	sch->shortcut(ShortcutIdentifier::SeekBwd).connect(this, [=](){
		play_manager->seek_rel_ms(-2000);
	});

	sch->shortcut(ShortcutIdentifier::SeekFwdFast).connect(this, [=]() {
		MilliSeconds ms = play_manager->duration_ms() / 20;
		play_manager->seek_rel_ms(ms);
	});

	sch->shortcut(ShortcutIdentifier::SeekBwdFast).connect(this, [=]() {
		MilliSeconds ms = play_manager->duration_ms() / 20;
		play_manager->seek_rel_ms(-ms);
	});
}


void GUI_ControlsBase::set_radio_mode(RadioMode radio)
{
	check_record_button_visible();

	if(radio != RadioMode::Off){
		buffering(0);
	}
}


MD::Interpretation GUI_ControlsBase::metadata_interpretation() const
{
	return MD::Interpretation::Tracks;
}

MetaDataList GUI_ControlsBase::info_dialog_data() const
{
	PlayState ps = PlayManager::instance()->playstate();
	if(ps == PlayState::Stopped){
		return MetaDataList();
	}

	return MetaDataList(current_track());
}

void GUI_ControlsBase::resizeEvent(QResizeEvent* e)
{
	Widget::resizeEvent(e);
	refresh_current_track();
}

void GUI_ControlsBase::showEvent(QShowEvent* e)
{
	Widget::showEvent(e);
	refresh_current_track();
}

void GUI_ControlsBase::contextMenuEvent(QContextMenuEvent* e)
{
	using Gui::LibraryContextMenu;

	if(!m->context_menu)
	{
		m->context_menu = new LibraryContextMenu(this);
		m->context_menu->show_actions
		(
			(LibraryContextMenu::EntryInfo |
			LibraryContextMenu::EntryLyrics |
			LibraryContextMenu::EntryEdit)
		);

		connect(m->context_menu, &LibraryContextMenu::sig_edit_clicked, this, [=](){
			show_edit();
		});

		connect(m->context_menu, &LibraryContextMenu::sig_info_clicked, this, [=](){
			show_info();
		});

		connect(m->context_menu, &LibraryContextMenu::sig_lyrics_clicked, this, [=](){
			show_lyrics();
		});

		m->context_menu->add_preference_action(new Gui::PlayerPreferencesAction(m->context_menu));
		m->context_menu->add_preference_action(new Gui::CoverPreferenceAction(m->context_menu));
	}

	m->context_menu->exec(e->globalPos());
}
