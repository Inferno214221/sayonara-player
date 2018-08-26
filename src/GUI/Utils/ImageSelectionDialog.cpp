#include "ImageSelectionDialog.h"

#include <QLayout>
#include <QLabel>
#include <QStringList>
#include <QDialog>

struct ImageSelectionDialog::Private
{
	QLabel* img_label=nullptr;
	QLabel* res_label=nullptr;

	Private(QWidget* parent)
	{
		img_label = new QLabel(parent);
		img_label->setMinimumSize(100, 100);
		img_label->setMaximumSize(100, 100);

		res_label = new QLabel(parent);
	}
};

ImageSelectionDialog::ImageSelectionDialog(const QString& dir, QWidget* parent) :
    Gui::WidgetTemplate<QFileDialog>(parent)
{
	m = Pimpl::make<Private>(this);

	QStringList filters;
	    filters << "*.jpg";
		filters << "*.png";
		filters << "*.gif";

	this->setDirectory(dir);
	this->setFilter(QDir::Files | QDir::Dirs);
	this->setLabelText(QFileDialog::DialogLabel::FileName, tr("Open image files"));
	this->setNameFilters(filters);
	this->setViewMode(QFileDialog::Detail);
	this->setModal(true);
	this->setAcceptMode(QFileDialog::AcceptOpen);
	QLayout* layout = this->layout();
	if(layout)
	{
		layout->addWidget(m->img_label);
		layout->addWidget(m->res_label);
	}

	connect(this, &QFileDialog::currentChanged, this, &ImageSelectionDialog::file_selected);
}

ImageSelectionDialog::~ImageSelectionDialog()
{

}

void ImageSelectionDialog::file_selected(const QString& file)
{
	QPixmap pm(file);
	if(pm.isNull()) {
		return;
	}

	m->img_label->setPixmap(
	    pm.scaled(m->img_label->size())
	);

	m->res_label->setText(
	    QString("%1x%2").arg(pm.width()).arg(pm.height())
	);
}
