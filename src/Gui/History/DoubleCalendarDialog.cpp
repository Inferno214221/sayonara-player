#include "DoubleCalendarDialog.h"

#include "Gui/Utils/Style.h"
#include "Gui/Utils/Widgets/CalendarWidget.h"
#include "Utils/Language/LanguageUtils.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QDate>
#include <QLocale>

using Gui::DoubleCalendarDialog;

struct DoubleCalendarDialog::Private
{
	Gui::CalendarWidget* calendarFrom=nullptr;
	Gui::CalendarWidget* calendarTo=nullptr;

	Gui::CalendarWidget* createCalendar(QWidget* parent)
	{
		auto* calendar = new Gui::CalendarWidget(parent);

		calendar->setSelectedDate(QDate::currentDate());
		calendar->setDateEditEnabled(false);
		calendar->setGridVisible(true);
		calendar->setFirstDayOfWeek(Qt::DayOfWeek::Monday);
		calendar->setLocale(Util::Language::getCurrentLocale());
		calendar->setHorizontalHeaderFormat(QCalendarWidget::HorizontalHeaderFormat::ShortDayNames);
		calendar->setMaximumDate(QDate::currentDate());
		calendar->showToday();
		calendar->resize(800, 600);


		return calendar;
	}
};

DoubleCalendarDialog::DoubleCalendarDialog(QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>();

	this->setWindowTitle(tr("Select date range"));
	this->setLayout( new QVBoxLayout() );
	this->setStyleSheet(Style::currentStyle());

	m->calendarFrom = m->createCalendar(this);
	m->calendarTo = m->createCalendar(this);

	m->calendarFrom->showToday();
	m->calendarTo->setMinimumDate(QDate::currentDate());
	m->calendarTo->showToday();

	{ // calendard layout
		auto* horLayout = new QHBoxLayout();
		horLayout->setSpacing(10);

		{
			auto* verLayout = new QVBoxLayout();
			verLayout->addWidget(new QLabel(tr("Start date"), this));
			verLayout->addWidget(m->calendarFrom);
			horLayout->addItem(verLayout);
		}

		{
			auto* verLayout = new QVBoxLayout();
			verLayout->addWidget(new QLabel(tr("End date"), this));
			verLayout->addWidget(m->calendarTo);
			horLayout->addItem(verLayout);
		}

		this->layout()->addItem(horLayout);
	}

	{ //
		auto* buttonBox = new QDialogButtonBox(this);
		buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		connect(buttonBox, &QDialogButtonBox::accepted, this, &DoubleCalendarDialog::sigAccepted);
		connect(buttonBox, &QDialogButtonBox::rejected, this, &DoubleCalendarDialog::sigRejected);
		connect(buttonBox, &QDialogButtonBox::accepted, this, &DoubleCalendarDialog::close);
		connect(buttonBox, &QDialogButtonBox::rejected, this, &DoubleCalendarDialog::close);

		this->layout()->addWidget(buttonBox);
	}

	connect(m->calendarFrom, &QCalendarWidget::clicked, this, &DoubleCalendarDialog::startDateSelected);
	connect(m->calendarTo, &QCalendarWidget::clicked, this, &DoubleCalendarDialog::endDateSelected);
}

DoubleCalendarDialog::~DoubleCalendarDialog() = default;

QDate DoubleCalendarDialog::startDate() const
{
	return m->calendarFrom->selectedDate();
}

QDate DoubleCalendarDialog::endDate() const
{
	return m->calendarTo->selectedDate();
}

void DoubleCalendarDialog::startDateSelected(const QDate& date)
{
	m->calendarTo->setMinimumDate(m->calendarFrom->selectedDate());
	m->calendarTo->setCurrentPage(date.year(), date.month());
	m->calendarTo->setSelectedDate(date);
}

void DoubleCalendarDialog::endDateSelected(const QDate& date)
{
	Q_UNUSED(date)
}
