#include "GUI_ControlsBase.h"
#include "SearchSlider.h"

#include "GUI/Covers/CoverButton.h"

#include "GUI/Utils/GuiUtils.h"
#include "GUI/Utils/Style.h"
#include "GUI/Utils/Icons.h"
#include "GUI/Utils/Shortcuts/Shortcut.h"
#include "GUI/Utils/Shortcuts/ShortcutHandler.h"
#include "GUI/Utils/PreferenceAction.h"
#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"
#include "GUI/Utils/Widgets/ProgressBar.h"
#include "GUI/Utils/RatingLabel.h"

#include "Components/PlayManager/PlayManager.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Engine/EngineHandler.h"
#include "Components/Tagging/ChangeNotifier.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Language.h"
#include "Utils/Logger/Logger.h"

#include <QToolTip>
#include <QImage>
#include <QPixmap>
#include <QIcon>
#include <QDateTime>
#include <QLabel>
#include <QFont>

#include <algorithm>

using Cover::Location;

struct GUI_ControlsBase::Private
{
	LibraryContextMenu* context_menu=nullptr;
};

GUI_ControlsBase::GUI_ControlsBase(QWidget* parent) :
	Gui::Widget(parent),
	ShortcutWidget()
{
	m = Pimpl::make<Private>();
}

GUI_ControlsBase::~GUI_ControlsBase() {}

void GUI_ControlsBase::init()
{
	QString version = _settings->get<Set::Player_Version>();

	lab_sayonara()->setText(tr("Sayonara Player"));
	lab_version()->setText( version );
	lab_writtenby()->setText(tr("Written by") + " Lucio Carreras");
	lab_copyright()->setText(tr("Copyright") + " 2011-" + QString::number(QDateTime::currentDateTime().date().year()));
	btn_rec()->setVisible(false);

	PlayManager* play_manager = PlayManager::instance();
	int volume = play_manager->volume();
	volume_changed(volume);

	bool muted = play_manager->is_muted();
	mute_changed(muted);

	setup_connections();
	setup_shortcuts();

	playstate_changed(play_manager->playstate());

	if(play_manager->playstate() != PlayState::Stopped)
	{
		track_changed(play_manager->current_track());
		cur_pos_changed(play_manager->initial_position_ms());
	}

	connect(btn_cover(), &CoverButton::sig_rejected, this, [=](){
		show_edit();
	});

	Set::listen<Set::Engine_SR_Active>(this, &GUI_ControlsBase::sr_active_changed);
	Set::listen<Set::Engine_Pitch>(this, &GUI_ControlsBase::file_info_changed);
	Set::listen<Set::Engine_SpeedActive>(this, &GUI_ControlsBase::file_info_changed, false);

	skin_changed();
}

RatingLabel*GUI_ControlsBase::lab_rating() const
{
	return nullptr;
}

// new track
void GUI_ControlsBase::track_changed(const MetaData & md)
{
	lab_sayonara()->hide();

	lab_title()->show();

	lab_version()->hide();
	lab_artist()->show();

	lab_writtenby()->hide();
	lab_album()->show();

	lab_copyright()->hide();

	lab_bitrate()->show();
	lab_filesize()->show();
	widget_details()->show();

	set_info_labels(md);
	set_cur_pos_label(0);
	set_total_time_label(md.length_ms);
	file_info_changed();
	set_cover_location(md);
	set_radio_mode( md.radio_mode() );

	sli_progress()->setEnabled( (md.length_ms / 1000) > 0 );

	if(lab_rating()){
		lab_rating()->setVisible(md.radio_mode() == RadioMode::Off);
		lab_rating()->set_rating(md.rating);
	}
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
	return;
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

	toggle_buffer_mode(false);

	lab_title()->hide();
	lab_sayonara()->show();

	lab_artist()->hide();
	lab_writtenby()->show();

	lab_album()->hide();
	lab_version()->show();

	widget_details()->hide();

	lab_copyright()->show();

	sli_progress()->setValue(0);
	sli_progress()->setEnabled(false);

	lab_current_time()->setText("00:00");
	lab_current_time()->hide();

	lab_max_time()->clear();
	lab_max_time()->setVisible(false);

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
	sli_buffer()->set_position(Gui::ProgressBar::Position::Middle);

	if(progress > 0 && progress < 100)
	{
		toggle_buffer_mode(true);

		sli_buffer()->setMinimum(0);
		sli_buffer()->setMaximum(100);
		sli_buffer()->setValue(progress);

		lab_current_time()->setText(QString("%1%").arg(progress));
		lab_max_time()->setVisible(false);
	}

	else if(progress == 0)
	{
		toggle_buffer_mode(false);

		sli_buffer()->setMinimum(0);
		sli_buffer()->setMaximum(0);
		sli_buffer()->setValue(progress);

		lab_current_time()->setText("0%");
		lab_max_time()->setVisible(false);
	}

	else
	{
		PlayManager* pm = PlayManager::instance();
		toggle_buffer_mode(false);

		sli_buffer()->setMinimum(0);
		sli_buffer()->setMaximum(0);

		lab_current_time()->clear();
		lab_max_time()->setVisible(pm->current_track().length_ms > 0);
	}
}

void GUI_ControlsBase::progress_moved(int val)
{
	val = std::max(val, 0);

	set_cur_pos_label(val);

	double percent = (val * 1.0) / sli_progress()->maximum();
	PlayManager::instance()->seek_rel(percent);
}


void GUI_ControlsBase::cur_pos_changed(MilliSeconds pos_ms)
{
	sp_log(Log::Crazy, this) << "Current position: " << pos_ms;

	MilliSeconds duration = PlayManager::instance()->duration_ms();
	int max = sli_progress()->maximum();
	int new_val;

	if ( duration > 0 ) {
		new_val = ( pos_ms * max ) / (duration);
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


void GUI_ControlsBase::set_cur_pos_label(int val)
{
	MilliSeconds duration = PlayManager::instance()->duration_ms();
	int max = sli_progress()->maximum();

	val = std::max(val, 0);
	val = std::min(max, val);

	double percent = (val * 1.0) / max;
	MilliSeconds cur_pos_ms =  (MilliSeconds) (percent * duration);
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
	MilliSeconds cur_pos_ms =  (MilliSeconds) (percent * duration);
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

	btn_mute()->setIcon(icon);
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
	if(val > 0){
		increase_volume();
	}
	else{
		decrease_volume();
	}
}

void GUI_ControlsBase::mute_changed(bool muted)
{
	int val;
	if(muted){
		val = 0;
	}

	else {
		val = PlayManager::instance()->volume();
	}

	sli_volume()->setValue(val);
	setup_volume_button(val);

	sli_volume()->setDisabled(muted);
}


// public slot:
// id3 tags have changed
void GUI_ControlsBase::id3_tags_changed(const MetaDataList& v_md_old, const MetaDataList& v_md_new)
{
	PlayManager* pm = PlayManager::instance();
	const MetaData& cur_md = pm->current_track();

	IdxList idxs = v_md_old.findTracks(cur_md.filepath());
	if(idxs.empty())
	{
		return;
	}

	const MetaData& md = v_md_new[idxs.first()];
	set_info_labels(md);
	set_cover_location(md);

	setWindowTitle(QString("Sayonara - ") + md.title());

	if(lab_rating())
	{
		lab_rating()->set_rating(md.rating);
	}
}

void GUI_ControlsBase::md_changed(const MetaData& md)
{
	MetaData modified_md(md);

	if(md.radio_mode() == RadioMode::Station){
		modified_md.set_album(md.album() + " (" + md.filepath() + ")");
	}

	set_info_labels(modified_md);
}

void GUI_ControlsBase::dur_changed(const MetaData& md)
{
	set_total_time_label(md.length_ms);
}

#include <cmath>
void GUI_ControlsBase::br_changed(const MetaData& md)
{
	if(md.bitrate / 1000 > 0){
	sp_log(Log::Debug, this) << "Bitrate " << md.bitrate;
		QString bitrate = QString::number(std::nearbyint(md.bitrate / 1000.)) + " kBit/s";
		lab_bitrate()->setText(bitrate);
	}

	if(md.filesize > 0)
	{
		QString filesize = QString::number( static_cast<double>(md.filesize / 1024) / 1024.0, 'f', 2) + " MB";
		lab_filesize()->setText(filesize);
	}
}

void GUI_ControlsBase::refresh_info_labels()
{
	set_info_labels(PlayManager::instance()->current_track());
}

void GUI_ControlsBase::set_info_labels(const MetaData& md)
{
	// title
	QString text = Gui::Util::elide_text(md.title(), lab_title(), 2);
	lab_title()->setText(text);

	//album
	QString str_year = QString::number(md.year);
	QString album_name(md.album());

	if(md.year > 1000 && !album_name.contains(str_year)){
		album_name += " (" + str_year + ")";
	}

	QString elided_text;
	QFontMetrics fm_album = lab_album()->fontMetrics();
	elided_text = fm_album.elidedText(album_name, Qt::ElideRight, lab_album()->width());
	lab_album()->setText(elided_text);

	//artist
	QFontMetrics fm_artist = lab_artist()->fontMetrics();
	elided_text = fm_artist.elidedText(md.artist(), Qt::ElideRight, lab_artist()->width());
	lab_artist()->setText(elided_text);
}

void GUI_ControlsBase::file_info_changed()
{
	const MetaData& md = PlayManager::instance()->current_track();

	QString rating_text;
	if( (_settings->get<Set::Engine_Pitch>() != 440) &&
		_settings->get<Set::Engine_SpeedActive>())
	{
		if(!rating_text.isEmpty()){
			rating_text += ", ";
		}

		rating_text += QString::number(_settings->get<Set::Engine_Pitch>()) + "Hz";
	}


	QString sBitrate;
	if(md.bitrate / 1000 > 0)
	{
		sBitrate = QString::number(md.bitrate / 1000) + " kBit/s";
		lab_bitrate()->setText(sBitrate);
	}

	lab_bitrate()->setVisible(!sBitrate.isEmpty());


	QString sFilesize;
	if(md.filesize > 0)
	{
		sFilesize = QString::number( static_cast<double>(md.filesize / 1024) / 1024.0, 'f', 2) + " MB";
		lab_filesize()->setText(sFilesize);
	}

	lab_filesize()->setVisible(!sFilesize.isEmpty());
}


void GUI_ControlsBase::skin_changed()
{
	Gui::Widget::skin_changed();

	using namespace Gui;

	btn_fwd()->setIcon(icon(Icons::Forward));
	btn_bwd()->setIcon(icon(Icons::Backward));

	if(PlayManager::instance()->playstate() == PlayState::Playing){
		btn_play()->setIcon(icon(Icons::Pause));
	}

	else{
		btn_play()->setIcon(icon(Icons::Play));
	}

	btn_stop()->setIcon(icon(Icons::Stop));
	btn_rec()->setIcon(icon(Icons::Record));

	setup_volume_button(sli_volume()->value());
}


void GUI_ControlsBase::sr_active_changed()
{
	check_record_button_visible();
	btn_rec()->setChecked(false);
}


void GUI_ControlsBase::check_record_button_visible()
{
	PlayManagerPtr play_manager = PlayManager::instance();

	const MetaData& md = play_manager->current_track();
	PlayState playstate = play_manager->playstate();

	bool is_lame_available = _settings->get<SetNoDB::MP3enc_found>();
	bool is_sr_active = _settings->get<Set::Engine_SR_Active>();
	bool is_radio = ((md.radio_mode() != RadioMode::Off));
	bool is_playing = (playstate == PlayState::Playing);

	bool recording_enabled = (is_lame_available &&
							  is_sr_active &&
							  is_radio &&
							  is_playing);

	btn_play()->setVisible(!recording_enabled);
	btn_rec()->setVisible(recording_enabled);

	if(!recording_enabled){
		btn_rec()->setChecked(false);
	}
}


void GUI_ControlsBase::set_cover_location(const MetaData& md)
{
	Location cl = Location::cover_location(md);

	btn_cover()->set_cover_location(cl);
}

void GUI_ControlsBase::set_standard_cover()
{
	btn_cover()->set_cover_location(Location::invalid_location());
}

void GUI_ControlsBase::force_cover(const QImage& img)
{
	btn_cover()->force_cover(img);
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

	connect(sli_volume(), &SearchSlider::sig_slider_moved, pm, &PlayManager::set_volume);
	connect(sli_progress(), &SearchSlider::sig_slider_moved, this, &GUI_ControlsBase::progress_moved);
	connect(sli_progress(), &SearchSlider::sig_slider_hovered, this, &GUI_ControlsBase::progress_hovered);

	connect(pm, &PlayManager::sig_playstate_changed, this, &GUI_ControlsBase::playstate_changed);
	connect(pm, &PlayManager::sig_track_changed, this, &GUI_ControlsBase::track_changed);
	connect(pm, &PlayManager::sig_position_changed_ms, this,	&GUI_ControlsBase::cur_pos_changed);
	connect(pm, &PlayManager::sig_buffer, this, &GUI_ControlsBase::buffering);
	connect(pm, &PlayManager::sig_volume_changed, this, &GUI_ControlsBase::volume_changed);
	connect(pm, &PlayManager::sig_mute_changed, this, &GUI_ControlsBase::mute_changed);
	connect(pm, &PlayManager::sig_record, this, &GUI_ControlsBase::rec_changed);

	// engine
	Engine::Handler* engine = Engine::Handler::instance();
	connect(engine, &Engine::Handler::sig_md_changed,	this, &GUI_ControlsBase::md_changed);
	connect(engine, &Engine::Handler::sig_duration_changed, this, &GUI_ControlsBase::dur_changed);
	connect(engine, &Engine::Handler::sig_bitrate_changed,	this, &GUI_ControlsBase::br_changed);
	connect(engine, &Engine::Handler::sig_cover_changed, this, &GUI_ControlsBase::force_cover);

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

	return MetaDataList(
		PlayManager::instance()->current_track()
	);
}

void GUI_ControlsBase::resizeEvent(QResizeEvent* e)
{
	Widget::resizeEvent(e);
	refresh_info_labels();
}

void GUI_ControlsBase::showEvent(QShowEvent* e)
{
	Widget::showEvent(e);
	refresh_info_labels();
}

void GUI_ControlsBase::contextMenuEvent(QContextMenuEvent* e)
{
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

		m->context_menu->add_preference_action(new PlayerPreferencesAction(m->context_menu));
		m->context_menu->add_preference_action(new CoverPreferenceAction(m->context_menu));
	}

	m->context_menu->exec(e->globalPos());
}
