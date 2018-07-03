#include "GUI_Controls.h"
#include "GUI/Player/ui_GUI_Controls.h"

GUI_Controls::GUI_Controls(QWidget* parent) :
	GUI_ControlsBase(parent)
{
	ui = new Ui::GUI_Controls();
	ui->setupUi(this);
}

GUI_Controls::~GUI_Controls()
{
	delete ui; ui=nullptr;
}

QLabel* GUI_Controls::lab_sayonara() const {	return ui->lab_sayonara; }
QLabel* GUI_Controls::lab_title() const { return ui->lab_title; }
QLabel* GUI_Controls::lab_version() const { return ui->lab_version; }
QLabel* GUI_Controls::lab_album() const { return ui->lab_album; }
QLabel* GUI_Controls::lab_artist() const { return ui->lab_artist; }
QLabel* GUI_Controls::lab_writtenby() const { return ui->lab_writtenby; }
QLabel* GUI_Controls::lab_bitrate() const { return ui->lab_bitrate; }
QLabel* GUI_Controls::lab_filesize() const { return ui->lab_filesize; }
QLabel* GUI_Controls::lab_copyright() const { return ui->lab_copyright; }
QLabel* GUI_Controls::lab_current_time() const { return ui->lab_cur_time; }
QLabel* GUI_Controls::lab_max_time() const { return ui->lab_max_time; }
QWidget* GUI_Controls::widget_details() const { return ui->widget_details; }
SearchSlider* GUI_Controls::sli_progress() const { return ui->sli_progress; }
SearchSlider* GUI_Controls::sli_volume() const { return ui->sli_volume; }
Gui::ProgressBar* GUI_Controls::sli_buffer() const { return ui->sli_buffer; }
QPushButton* GUI_Controls::btn_mute() const { return ui->btn_mute; }
QPushButton* GUI_Controls::btn_play() const { return ui->btn_play; }
QPushButton* GUI_Controls::btn_rec() const { return ui->btn_rec; }
QPushButton* GUI_Controls::btn_bwd() const { return ui->btn_bw; }
QPushButton* GUI_Controls::btn_fwd() const { return ui->btn_fw; }
QPushButton* GUI_Controls::btn_stop() const { return ui->btn_stop; }
CoverButton* GUI_Controls::btn_cover() const { return ui->btn_cover; }

void GUI_Controls::toggle_buffer_mode(bool buffering)
{
	if(!buffering){
		ui->progress_widget->setCurrentIndex(0);
	}

	else {
		ui->progress_widget->setCurrentIndex(1);
	}
}

bool GUI_Controls::is_resizable() const
{
	return false;
}
