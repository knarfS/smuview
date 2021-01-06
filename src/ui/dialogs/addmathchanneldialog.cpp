/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cassert>
#include <memory>
#include <set>
#include <string>

#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSizePolicy>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "addmathchanneldialog.hpp"
#include "src/channels/addscchannel.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/dividechannel.hpp"
#include "src/channels/integratechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/channels/movingavgchannel.hpp"
#include "src/channels/multiplysfchannel.hpp"
#include "src/channels/multiplysschannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/data/quantitycombobox.hpp"
#include "src/ui/data/quantityflagslist.hpp"
#include "src/ui/data/unitcombobox.hpp"
#include "src/ui/devices/channelgroupcombobox.hpp"
#include "src/ui/devices/devicecombobox.hpp"
#include "src/ui/devices/selectsignalwidget.hpp"

using std::make_shared;
using std::set;
using std::static_pointer_cast;
using std::string;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace ui {
namespace dialogs {

AddMathChannelDialog::AddMathChannelDialog(const Session &session,
		shared_ptr<sv::devices::BaseDevice> device,
		QWidget *parent) :
	QDialog(parent),
	session_(session),
	device_(device)
{
	assert(device);

	setup_ui();
}

void AddMathChannelDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("Add Math Channel"));
	this->setMinimumWidth(550);

	QVBoxLayout *main_layout = new QVBoxLayout();

	// General stuff
	QFormLayout *form_layout = new QFormLayout();
	name_edit_ = new QLineEdit();
	form_layout->addRow(tr("Name"), name_edit_);
	main_layout->addLayout(form_layout);

	// Measured Quantity
	QGroupBox *mq_group = new QGroupBox(tr("Measured Quantity"));
	QFormLayout *mq_layout = new QFormLayout();
	quantity_box_ = new ui::data::QuantityComboBox();
	mq_layout->addRow(tr("Quantity"), quantity_box_);
	quantity_flags_list_ = new ui::data::QuantityFlagsList();
	mq_layout->addRow(tr("Quantity Flags"), quantity_flags_list_);
	unit_box_ = new ui::data::UnitComboBox();
	mq_layout->addRow(tr("Unit"), unit_box_);
	mq_group->setLayout(mq_layout);
	main_layout->addWidget(mq_group);

	// Add to...
	QGroupBox *add_to_group = new QGroupBox(tr("Add to..."));
	QFormLayout *add_to_layout = new QFormLayout();
	device_box_ = new ui::devices::DeviceComboBox(session_);
	device_box_->select_device(device_);
	add_to_layout->addRow(tr("Device"), device_box_);
	channel_group_box_ = new ui::devices::ChannelGroupComboBox(device_);
	channel_group_box_->addItem(QString(tr("Math")));
	connect(device_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_device_changed()));
	add_to_layout->addRow(tr("Channel Group"), channel_group_box_);
	add_to_group->setLayout(add_to_layout);
	main_layout->addWidget(add_to_group);

	// Tabs
	tab_widget_ = new QTabWidget();
	this->setup_ui_multiply_signals_tab();
	this->setup_ui_multiply_signal_tab();
	this->setup_ui_divide_signals_tab();
	this->setup_ui_add_signal_tab();
	this->setup_ui_integrate_signal_tab();
	this->setup_ui_movingavg_signal_tab();
	tab_widget_->setCurrentIndex(0);
	main_layout->addWidget(tab_widget_);

	// Buttons
	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(main_layout);
}

void AddMathChannelDialog::setup_ui_multiply_signals_tab()
{
	QString title(tr("S\u2081(t) * S\u2082(t)"));

	QWidget *widget = new QWidget();
	QHBoxLayout *layout = new QHBoxLayout();

	QGroupBox *signal1_group = new QGroupBox(tr("Signal 1"));
	QVBoxLayout *s1_layout = new QVBoxLayout();
	m_ss_signal1_ = new ui::devices::SelectSignalWidget(session_);
	m_ss_signal1_->select_device(device_);
	s1_layout->addWidget(m_ss_signal1_);
	signal1_group->setLayout(s1_layout);
	layout->addWidget(signal1_group);

	QGroupBox *signal2_group = new QGroupBox(tr("Signal 2"));
	QVBoxLayout *s2_layout = new QVBoxLayout();
	m_ss_signal2_ = new ui::devices::SelectSignalWidget(session_);
	m_ss_signal2_->select_device(device_);
	s2_layout->addWidget(m_ss_signal2_);
	signal2_group->setLayout(s2_layout);
	layout->addWidget(signal2_group);

	widget->setLayout(layout);
	tab_widget_->addTab(widget, title);
}

void AddMathChannelDialog::setup_ui_multiply_signal_tab()
{
	QString title(tr("S(t) * f"));

	QWidget *widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();

	QGroupBox *signal_group = new QGroupBox(tr("Signal"));
	QVBoxLayout *s_layout = new QVBoxLayout();
	m_sf_signal_ = new ui::devices::SelectSignalWidget(session_);
	m_sf_signal_->select_device(device_);
	s_layout->addWidget(m_sf_signal_);
	signal_group->setLayout(s_layout);
	layout->addWidget(signal_group);

	QFormLayout *f_layout = new QFormLayout();
	m_sf_factor_edit_ = new QLineEdit();
	f_layout->addRow(tr("Factor"), m_sf_factor_edit_);
	layout->addLayout(f_layout);

	widget->setLayout(layout);
	tab_widget_->addTab(widget, title);
}

void AddMathChannelDialog::setup_ui_divide_signals_tab()
{
	QString title(tr("S\u2081(t) / S\u2082(t)"));

	QWidget *widget = new QWidget();
	QHBoxLayout *layout = new QHBoxLayout();

	QGroupBox *signal1_group = new QGroupBox(tr("Signal 1"));
	QVBoxLayout *s1_layout = new QVBoxLayout();
	d_ss_signal1_ = new ui::devices::SelectSignalWidget(session_);
	d_ss_signal1_->select_device(device_);
	s1_layout->addWidget(d_ss_signal1_);
	signal1_group->setLayout(s1_layout);
	layout->addWidget(signal1_group);

	QGroupBox *signal2_group = new QGroupBox(tr("Signal 2"));
	QVBoxLayout *s2_layout = new QVBoxLayout();
	d_ss_signal2_ = new ui::devices::SelectSignalWidget(session_);
	d_ss_signal2_->select_device(device_);
	s2_layout->addWidget(d_ss_signal2_);
	signal2_group->setLayout(s2_layout);
	layout->addWidget(signal2_group);

	widget->setLayout(layout);
	tab_widget_->addTab(widget, title);
}

void AddMathChannelDialog::setup_ui_add_signal_tab()
{
	QString title(tr("S(t) + c"));

	QWidget *widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();

	QGroupBox *signal_group = new QGroupBox(tr("Signal"));
	QVBoxLayout *s_layout = new QVBoxLayout();
	a_sc_signal_ = new ui::devices::SelectSignalWidget(session_);
	a_sc_signal_->select_device(device_);
	s_layout->addWidget(a_sc_signal_);
	signal_group->setLayout(s_layout);
	layout->addWidget(signal_group);

	QFormLayout *c_layout = new QFormLayout();
	a_sc_constant_edit_ = new QLineEdit();
	c_layout->addRow(tr("Constant"), a_sc_constant_edit_);
	layout->addLayout(c_layout);

	widget->setLayout(layout);
	tab_widget_->addTab(widget, title);
}

void AddMathChannelDialog::setup_ui_integrate_signal_tab()
{
	QString title(tr("\u222B S(t) * dt"));

	QWidget *widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();

	QGroupBox *signal_group = new QGroupBox(tr("Signal"));
	QVBoxLayout *s_layout = new QVBoxLayout();
	i_s_signal_ = new ui::devices::SelectSignalWidget(session_);
	i_s_signal_->select_device(device_);
	s_layout->addWidget(i_s_signal_);
	signal_group->setLayout(s_layout);
	layout->addWidget(signal_group);

	widget->setLayout(layout);
	tab_widget_->addTab(widget, title);
}

void AddMathChannelDialog::setup_ui_movingavg_signal_tab()
{
	QString title(tr("Moving Average"));

	QWidget *widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();

	QGroupBox *signal_group = new QGroupBox(tr("Signal"));
	QVBoxLayout *s_layout = new QVBoxLayout();
	ma_signal_ = new ui::devices::SelectSignalWidget(session_);
	ma_signal_->select_device(device_);
	s_layout->addWidget(ma_signal_);
	signal_group->setLayout(s_layout);
	layout->addWidget(signal_group);

	QFormLayout *ac_layout = new QFormLayout();
	ma_num_samples_box_ = new QSpinBox();
	ma_num_samples_box_->setMinimum(1);
	ac_layout->addRow(tr("Sample count"), ma_num_samples_box_);
	layout->addLayout(ac_layout);

	widget->setLayout(layout);
	tab_widget_->addTab(widget, title);
}

shared_ptr<channels::MathChannel> AddMathChannelDialog::channel() const
{
	return channel_;
}

QString AddMathChannelDialog::channel_group_name() const
{
	return channel_group_box_->selected_channel_group();
}

void AddMathChannelDialog::accept()
{
	if (name_edit_->text().size() == 0) {
		QMessageBox::warning(this,
			tr("Channel name missing"),
			tr("Please enter a name for the new channel."),
			QMessageBox::Ok);
		return;
	}

	auto device = device_box_->selected_device();
	string chg_name = channel_group_name().toStdString();
	set<string> channel_group_names { chg_name };

	auto quantity = quantity_box_->selected_quantity();
	auto quantity_flags = quantity_flags_list_->selected_quantity_flags();
	auto unit = unit_box_->selected_unit();

	switch (tab_widget_->currentIndex()) {
	case 0: {
			if (m_ss_signal1_->selected_signal() == nullptr) {
				QMessageBox::warning(this,
					tr("Signal missing"),
					tr("Please choose signal 1 for the signal multiplication."),
					QMessageBox::Ok);
				return;
			}
			auto signal_1 = static_pointer_cast<sv::data::AnalogTimeSignal>(
				m_ss_signal1_->selected_signal());

			if (m_ss_signal2_->selected_signal() == nullptr) {
				QMessageBox::warning(this,
					tr("Signal missing"),
					tr("Please choose signal 2 for the signal multiplication."),
					QMessageBox::Ok);
				return;
			}
			auto signal_2 = static_pointer_cast<sv::data::AnalogTimeSignal>(
				m_ss_signal2_->selected_signal());

			double start_timestamp = signal_1->signal_start_timestamp();
			if (signal_2->signal_start_timestamp() < start_timestamp)
				start_timestamp = signal_2->signal_start_timestamp();

			channel_ = make_shared<channels::MultiplySSChannel>(
				quantity, quantity_flags, unit,
				signal_1, signal_2,
				device, channel_group_names, name_edit_->text().toStdString(),
				start_timestamp);
		}
		break;
	case 1: {
			if (m_sf_signal_->selected_signal() == nullptr) {
				QMessageBox::warning(this,
					tr("Signal missing"),
					tr("Please choose a signal for the factor multiplication."),
					QMessageBox::Ok);
				return;
			}
			auto signal = static_pointer_cast<sv::data::AnalogTimeSignal>(
				m_sf_signal_->selected_signal());

			if (m_sf_factor_edit_->text().size() == 0) {
				QMessageBox::warning(this,
					tr("Factor missing"),
					tr("Please enter a factor for the factor multiplication."),
					QMessageBox::Ok);
				return;
			}

			bool ok;
			double factor = QString(m_sf_factor_edit_->text()).toDouble(&ok);
			if (!ok) {
				QMessageBox::warning(this,
					tr("Factor not a number"),
					tr("Please enter a number as factor for the factor multiplication."),
					QMessageBox::Ok);
				return;
			}

			channel_ = make_shared<channels::MultiplySFChannel>(
				quantity, quantity_flags, unit,
				signal, factor,
				device, channel_group_names, name_edit_->text().toStdString(),
				signal->signal_start_timestamp());
		}
		break;
	case 2: {
			if (d_ss_signal1_->selected_signal() == nullptr) {
				QMessageBox::warning(this,
					tr("Signal missing"),
					tr("Please choose signal 1 for the signal division."),
					QMessageBox::Ok);
				return;
			}
			auto signal1 = static_pointer_cast<sv::data::AnalogTimeSignal>(
				d_ss_signal1_->selected_signal());

			if (d_ss_signal2_->selected_signal() == nullptr) {
				QMessageBox::warning(this,
					tr("Signal missing"),
					tr("Please choose signal 2 for the signal division."),
					QMessageBox::Ok);
				return;
			}
			auto signal2 = static_pointer_cast<sv::data::AnalogTimeSignal>(
				d_ss_signal2_->selected_signal());

			double start_timestamp = signal1->signal_start_timestamp();
			if (signal2->signal_start_timestamp() < start_timestamp)
				start_timestamp = signal2->signal_start_timestamp();

			channel_ = make_shared<channels::DivideChannel>(
				quantity, quantity_flags, unit,
				signal1, signal2,
				device, channel_group_names, name_edit_->text().toStdString(),
				start_timestamp);
		}
		break;
	case 3: {
			if (a_sc_signal_->selected_signal() == nullptr) {
				QMessageBox::warning(this,
					tr("Signal missing"),
					tr("Please choose a signal for the constant addition."),
					QMessageBox::Ok);
				return;
			}
			auto signal = static_pointer_cast<sv::data::AnalogTimeSignal>(
				a_sc_signal_->selected_signal());

			if (a_sc_constant_edit_->text().size() == 0) {
				QMessageBox::warning(this,
					tr("Constant missing"),
					tr("Please enter a constant for the constant addition."),
					QMessageBox::Ok);
				return;
			}

			bool ok;
			double constant = QString(a_sc_constant_edit_->text()).toDouble(&ok);
			if (!ok) {
				QMessageBox::warning(this,
					tr("Constant not a number"),
					tr("Please enter a number as constant for the constant addition."),
					QMessageBox::Ok);
				return;
			}

			channel_ = make_shared<channels::AddSCChannel>(
				quantity, quantity_flags, unit,
				signal, constant,
				device, channel_group_names, name_edit_->text().toStdString(),
				signal->signal_start_timestamp());
		}
		break;
	case 4: {
			if (i_s_signal_->selected_signal() == nullptr) {
				QMessageBox::warning(this,
					tr("Signal missing"),
					tr("Please choose a signal for the integration."),
					QMessageBox::Ok);
				return;
			}
			auto signal = static_pointer_cast<sv::data::AnalogTimeSignal>(
				i_s_signal_->selected_signal());

			channel_ = make_shared<channels::IntegrateChannel>(
				quantity, quantity_flags, unit,
				signal,
				device, channel_group_names, name_edit_->text().toStdString(),
				signal->signal_start_timestamp());
		}
		break;
	case 5: {
			if (ma_signal_->selected_signal() == nullptr) {
				QMessageBox::warning(this,
					tr("Signal missing"),
					tr("Please choose a signal for the moving average."),
					QMessageBox::Ok);
				return;
			}
			auto signal = static_pointer_cast<sv::data::AnalogTimeSignal>(
				ma_signal_->selected_signal());

			uint num_samples = ma_num_samples_box_->value();

			channel_ = make_shared<channels::MovingAvgChannel>(
				quantity, quantity_flags, unit,
				signal, num_samples,
				device, channel_group_names, name_edit_->text().toStdString(),
				signal->signal_start_timestamp());
		}
		break;
	default:
		break;
	}

	QDialog::accept();
}

void AddMathChannelDialog::on_device_changed()
{
	channel_group_box_->change_device(device_box_->selected_device());
}

} // namespace dialogs
} // namespace ui
} // namespace sv
