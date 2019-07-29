#include "GUI_FailMessageBox.h"
#include "Gui/TagEdit/ui_GUI_FailMessageBox.h"

#include "Utils/Language/Language.h"
#include <QVBoxLayout>

GUI_FailMessageBox::GUI_FailMessageBox(QWidget* parent) :
	Gui::Dialog(parent)
{
	ui = new Ui::GUI_FailMessageBox();
	ui->setupUi(this);
	ui->tv_files->setVisible(false);

	connect(ui->cb_details, &QCheckBox::toggled, this, &GUI_FailMessageBox::details_toggled);
	connect(ui->btn_ok, &QPushButton::clicked, this, &GUI_FailMessageBox::close);

	language_changed();
}

GUI_FailMessageBox::~GUI_FailMessageBox()
{
	delete ui; ui=nullptr;
}

void GUI_FailMessageBox::set_failed_files(const QMap<QString, Tagging::Editor::FailReason>& files)
{
	ui->tv_files->clear();

	using Reason=Tagging::Editor::FailReason;
	ui->tv_files->setRowCount(files.size());
	ui->tv_files->setColumnCount(3);
	ui->tv_files->setHorizontalHeaderLabels(QStringList{
		Lang::get(Lang::Filename),
		tr("File exists") + "?",
		tr("Writeable") + "?"
	});

	const QList<QString> keys = files.keys();
	int row=0;
	for(const QString& key : keys)
	{
		Reason reason = files[key];

		auto twi_filename = new QTableWidgetItem(key);
		twi_filename->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

		auto twi_exists = new QTableWidgetItem();
		twi_exists->setTextAlignment(Qt::AlignCenter);

		auto twi_writable = new QTableWidgetItem();
		twi_writable->setTextAlignment(Qt::AlignCenter);

		if(reason == Reason::FileNotFound)
		{
			twi_exists->setText(Lang::get(Lang::No));
			twi_writable->setText(Lang::get(Lang::No));
		}

		else if(reason == Reason::FileNotWriteable)
		{
			twi_exists->setText(Lang::get(Lang::Yes));
			twi_writable->setText(Lang::get(Lang::No));
		}

		else
		{
			twi_exists->setText(Lang::get(Lang::Yes));
			twi_writable->setText(Lang::get(Lang::Yes));
		}

		ui->tv_files->setItem(row, 0, twi_filename);
		ui->tv_files->setItem(row, 1, twi_exists);
		ui->tv_files->setItem(row, 2, twi_writable);

		ui->tv_files->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

		row++;
	}
}

void GUI_FailMessageBox::details_toggled(bool b)
{
	ui->tv_files->setVisible(b);

	if(b) {
		this->adjustSize();
		QSize sz = this->size();
		sz.setWidth(parentWidget()->width());
		this->resize(sz);
	}

	else {
		this->adjustSize();
	}
}

void GUI_FailMessageBox::language_changed()
{
	ui->lab_warning->setText(tr("Some files could not be saved"));
	ui->lab_header->setText(Lang::get(Lang::Warning));
}

void GUI_FailMessageBox::showEvent(QShowEvent* e)
{
	Gui::Dialog::showEvent(e);
	this->adjustSize();
}
