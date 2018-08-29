/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <cassert>
#include <memory>

#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QMessageBox>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "addmathchanneldialog.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/dividechannel.hpp"
#include "src/channels/integratechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/channels/multiplysfchannel.hpp"
#include "src/channels/multiplysschannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/widgets/channelgroupcombobox.hpp"
#include "src/widgets/devicecombobox.hpp"
#include "src/widgets/quantitycombobox.hpp"
#include "src/widgets/quantityflagslist.hpp"
#include "src/widgets/signaltree.hpp"
#include "src/widgets/unitcombobox.hpp"

using std::make_shared;
using std::static_pointer_cast;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace dialogs {

AddMathChannelDialog::AddMathChannelDialog(const Session &session,
		shared_ptr<devices::BaseDevice> device,
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
	QIcon mainIcon;
	mainIcon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(mainIcon);
	this->setWindowTitle(tr("Add Math Channel"));
	this->setMinimumWidth(550);

	QVBoxLayout *main_layout = new QVBoxLayout();

	// General stuff
	QFormLayout *form_layout = new QFormLayout();
	name_edit_ = new QLineEdit();
	form_layout->addRow(tr("Name"), name_edit_);
	quantity_box_ = new widgets::QuantityComboBox();
	form_layout->addRow(tr("Quantity"), quantity_box_);
	quantity_flags_list_ = new widgets::QuantityFlagsList();
	form_layout->addRow(tr("Quantity Flags"), quantity_flags_list_);
	unit_box_ = new widgets::UnitComboBox();
	form_layout->addRow(tr("Unit"), unit_box_);
	device_box_ = new widgets::DeviceComboBox(session_);
	device_box_->select_device(device_);
	form_layout->addRow(tr("Device"), device_box_);
	channel_group_box_ = new widgets::ChannelGroupComboBox(session_, device_);
	connect(device_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_device_changed()));
	form_layout->addRow(tr("Channel Group"), channel_group_box_);

	main_layout->addLayout(form_layout);

	// Tabs
	tab_widget_ = new QTabWidget();
	this->setup_ui_multiply_signals_tab();
	this->setup_ui_multiply_signal_tab();
	this->setup_ui_divide_signals_tab();
	this->setup_ui_integrate_signal_tab();
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
	QFormLayout *form_layout = new QFormLayout();

	m_ss_signal_1_tree_ = new widgets::SignalTree(
		session_, true, true, false, device_);
	form_layout->addRow(tr("Signal 1"), m_ss_signal_1_tree_);

	m_ss_signal_2_tree_ = new widgets::SignalTree(
		session_, true, true, false, device_);
	form_layout->addRow(tr("Signal 2"), m_ss_signal_2_tree_);

	widget->setLayout(form_layout);
	tab_widget_->addTab(widget, title);
}

void AddMathChannelDialog::setup_ui_multiply_signal_tab()
{
	QString title(tr("S(t) * f"));

	QWidget *widget = new QWidget();
	QFormLayout *form_layout = new QFormLayout();

	m_sf_signal_tree_ = new widgets::SignalTree(
		session_, true, true, false, device_);
	// Workaround to vert. maximize the tree in the form layout
	QSizePolicy policy = m_sf_signal_tree_->sizePolicy();
	policy.setVerticalStretch(1);
	m_sf_signal_tree_->setSizePolicy(policy);
	form_layout->addRow(tr("Signal"), m_sf_signal_tree_);

	m_sf_factor_edit_ = new QLineEdit();
	form_layout->addRow(tr("Factor"), m_sf_factor_edit_);

	widget->setLayout(form_layout);
	tab_widget_->addTab(widget, title);
}

void AddMathChannelDialog::setup_ui_divide_signals_tab()
{
	QString title(tr("S\u2081(t) / S\u2082(t)"));

	QWidget *widget = new QWidget();
	QFormLayout *form_layout = new QFormLayout();

	d_ss_signal_1_tree_ = new widgets::SignalTree(
		session_, true, true, false, device_);
	form_layout->addRow(tr("Signal 1"), d_ss_signal_1_tree_);

	d_ss_signal_2_tree_ = new widgets::SignalTree(
		session_, true, true, false, device_);
	form_layout->addRow(tr("Signal 2"), d_ss_signal_2_tree_);

	widget->setLayout(form_layout);
	tab_widget_->addTab(widget, title);
}

void AddMathChannelDialog::setup_ui_integrate_signal_tab()
{
	QString title(tr("\u222B S(t) * dt"));

	QWidget *widget = new QWidget();
	QFormLayout *form_layout = new QFormLayout();

	i_s_signal_tree_ = new widgets::SignalTree(
		session_, true, true, false, device_);
	// Workaround to vert. maximize the tree in the form layout
	QSizePolicy policy = i_s_signal_tree_->sizePolicy();
	policy.setVerticalStretch(1);
	i_s_signal_tree_->setSizePolicy(policy);
	form_layout->addRow(tr("Signal"), i_s_signal_tree_);

	widget->setLayout(form_layout);
	tab_widget_->addTab(widget, title);
}

vector<shared_ptr<channels::BaseChannel>> AddMathChannelDialog::channels()
{
	return channels_;
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
	QString channel_group_name = channel_group_box_->selected_channel_group();

	shared_ptr<channels::MathChannel> channel;
	switch (tab_widget_->currentIndex()) {
	case 0: {
			shared_ptr<data::AnalogSignal> signal_1;
			auto signals_1 = m_ss_signal_1_tree_->selected_signals();
			if (signals_1.size() != 1) {
				QMessageBox::warning(this,
					tr("Signal 1 missing"),
					tr("Please choose a signal 1 for the new channel."),
					QMessageBox::Ok);
				return;
			}
			signal_1 = static_pointer_cast<data::AnalogSignal>(signals_1[0]);

			shared_ptr<data::AnalogSignal> signal_2;
			auto signals_2 = m_ss_signal_2_tree_->selected_signals();
			if (signals_2.size() != 1) {
				QMessageBox::warning(this,
					tr("Signal 2 missing"),
					tr("Please choose a signal 2 for the new channel."),
					QMessageBox::Ok);
				return;
			}
			signal_2 = static_pointer_cast<data::AnalogSignal>(signals_2[0]);

			double start_timestamp = signal_1->signal_start_timestamp();
			if (signal_2->signal_start_timestamp() < start_timestamp)
				start_timestamp = signal_2->signal_start_timestamp();

			channel = make_shared<channels::MultiplySSChannel>(
				quantity_box_->selected_quantity(),
				quantity_flags_list_->selected_quantity_flags(),
				unit_box_->selected_unit(),
				signal_1, signal_2,
				device, channel_group_name, name_edit_->text(),
				start_timestamp);
		}
		break;
	case 1: {
			shared_ptr<data::AnalogSignal> signal;
			auto signals = m_sf_signal_tree_->selected_signals();
			if (signals.size() != 1) {
				QMessageBox::warning(this,
					tr("Signal missing"),
					tr("Please choose a signal for the new channel."),
					QMessageBox::Ok);
				return;
			}
			signal = static_pointer_cast<data::AnalogSignal>(signals[0]);

			if (m_sf_factor_edit_->text().size() == 0) {
				QMessageBox::warning(this,
					tr("Factor missing"),
					tr("Please enter a factor for the new channel."),
					QMessageBox::Ok);
				return;
			}

			bool ok;
			double factor = QString(m_sf_factor_edit_->text()).toDouble(&ok);
			if (!ok) {
				QMessageBox::warning(this,
					tr("Factor not a number"),
					tr("Please enter a number as factor for the new channel."),
					QMessageBox::Ok);
				return;
			}

			channel = make_shared<channels::MultiplySFChannel>(
				quantity_box_->selected_quantity(),
				quantity_flags_list_->selected_quantity_flags(),
				unit_box_->selected_unit(),
				signal, factor,
				device, channel_group_name, name_edit_->text(),
				signal->signal_start_timestamp());
		}
		break;
	case 2: {
			shared_ptr<data::AnalogSignal> signal_1;
			auto signals_1 = d_ss_signal_1_tree_->selected_signals();
			if (signals_1.size() != 1) {
				QMessageBox::warning(this,
					tr("Signal 1 missing"),
					tr("Please choose a signal 1 for the new channel."),
					QMessageBox::Ok);
				return;
			}
			signal_1 = static_pointer_cast<data::AnalogSignal>(signals_1[0]);

			shared_ptr<data::AnalogSignal> signal_2;
			auto signals_2 = d_ss_signal_2_tree_->selected_signals();
			if (signals_2.size() != 1) {
				QMessageBox::warning(this,
					tr("Signal 2 missing"),
					tr("Please choose a signal 2 for the new channel."),
					QMessageBox::Ok);
				return;
			}
			signal_2 = static_pointer_cast<data::AnalogSignal>(signals_2[0]);

			double start_timestamp = signal_1->signal_start_timestamp();
			if (signal_2->signal_start_timestamp() < start_timestamp)
				start_timestamp = signal_2->signal_start_timestamp();

			channel = make_shared<channels::DivideChannel>(
				quantity_box_->selected_quantity(),
				quantity_flags_list_->selected_quantity_flags(),
				unit_box_->selected_unit(),
				signal_1, signal_2,
				device, channel_group_name, name_edit_->text(),
				start_timestamp);
		}
		break;
	case 3: {
			shared_ptr<data::AnalogSignal> signal;
			auto signals_i = i_s_signal_tree_->selected_signals();
			if (signals_i.size() != 1) {
				QMessageBox::warning(this,
					tr("Signal missing"),
					tr("Please choose a signal for the new channel."),
					QMessageBox::Ok);
				return;
			}
			signal = static_pointer_cast<data::AnalogSignal>(signals_i[0]);

			channel = make_shared<channels::IntegrateChannel>(
				quantity_box_->selected_quantity(),
				quantity_flags_list_->selected_quantity_flags(),
				unit_box_->selected_unit(),
				signal,
				device, channel_group_name, name_edit_->text(),
				signal->signal_start_timestamp());
		}
		break;
	default:
		break;
	}

	channel->init_signal();
	channels_.push_back(channel);

	QDialog::accept();
}

void AddMathChannelDialog::on_device_changed()
{
	channel_group_box_->change_device(device_box_->selected_device());
}

} // namespace dialogs
} // namespace sv
