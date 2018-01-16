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

#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "addmathchanneldialog.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/channels/multiplychannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/devices/device.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/widgets/quantitycombobox.hpp"
#include "src/widgets/signaltree.hpp"

using std::static_pointer_cast;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace dialogs {

AddMathChannelDialog::AddMathChannelDialog(const Session &session,
		const shared_ptr<devices::HardwareDevice> device,
		QWidget *parent) :
	QDialog(parent),
	session_(session),
	device_(device)
{
	setup_ui();
}

void AddMathChannelDialog::setup_ui()
{
	this->setWindowTitle(tr("Add View"));

	QVBoxLayout *main_layout = new QVBoxLayout;

	tab_widget_ = new QTabWidget();
	this->setup_ui_multiply_tab();
	this->setup_ui_divide_tab();
	this->setup_ui_integrate_tab();
	//tab_widget_->setCurrentIndex(selected_view_type_);
	main_layout->addWidget(tab_widget_);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(main_layout);
}

void AddMathChannelDialog::setup_ui_multiply_tab()
{
	QString title(tr("Multiply"));
	QWidget *multiply_widget = new QWidget();

	QVBoxLayout *main_layout = new QVBoxLayout;

	multiply_signal_1_tree_ = new widgets::SignalTree(
		session_, true, false, device_);
	main_layout->addWidget(multiply_signal_1_tree_);

	multiply_signal_2_tree_ = new widgets::SignalTree(
		session_, true, false, device_);
	main_layout->addWidget(multiply_signal_2_tree_);
	multiply_widget->setLayout(main_layout);

	QFormLayout *form_layout = new QFormLayout();

	widgets::QuantityComboBox *quantity_box_ =
		new widgets::QuantityComboBox(session_);
	form_layout->addRow(tr("Quantity"), quantity_box_);

	main_layout->addLayout(form_layout);

	tab_widget_->addTab(multiply_widget, title);
}

void AddMathChannelDialog::setup_ui_divide_tab()
{
	QString title(tr("Divide"));
	QWidget *divide_widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();
	divide_widget->setLayout(layout);

	tab_widget_->addTab(divide_widget, title);
}

void AddMathChannelDialog::setup_ui_integrate_tab()
{
	QString title(tr("Integrate"));
	QWidget *integrate_widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();
	integrate_widget->setLayout(layout);

	tab_widget_->addTab(integrate_widget, title);
}

vector<shared_ptr<channels::BaseChannel>> AddMathChannelDialog::channels()
{
	return channels_;
}

void AddMathChannelDialog::accept()
{
	shared_ptr<devices::Channel> channel;
	int tab_index = tab_widget_->currentIndex();
	switch (tab_index) {
	case 0: {
			shared_ptr<data::AnalogSignal> signal_1;
			auto signals_1 = multiply_signal_1_tree_->selected_signals();
			if (signals_1.size() != 1)
				return;
			signal_1 = static_pointer_cast<data::AnalogSignal>(signals_1[0]);

			shared_ptr<data::AnalogSignal> signal_2;
			auto signals_2 = multiply_signal_2_tree_->selected_signals();
			if (signals_2.size() != 1)
				return;
			signal_2 = static_pointer_cast<data::AnalogSignal>(signals_2[0]);

			auto channel = make_shared<channels::MultiplyChannel>(
				sigrok::Quantity::POWER,
				vector<const sigrok::QuantityFlag *>(),
				sigrok::Unit::WATT,
				signal_1, signal_2,
				QString("Math 1"), QString("CHG 1"),
				signal_1->signal_start_timestamp());

			channels_.push_back(channel);
		}
		break;
	case 1:
		/*
		for (auto channel : panel_channel_tree_->selected_channels()) {
			views_.push_back(
				make_shared<views::ValuePanelView>(session_, channel));
		}
		*/
		break;
	case 2:
		/*
		for (auto channel : plot_channel_tree_->selected_channels()) {
			views_.push_back(make_shared<views::PlotView>(session_, channel));
		}
		for (auto signal : plot_channel_tree_->selected_signals()) {
			//TODO
			auto a_signal = static_pointer_cast<data::AnalogSignal>(signal);
			views_.push_back(make_shared<views::PlotView>(session_, a_signal));
		}
		*/
		break;
	default:
		break;
	}

	QDialog::accept();
}

} // namespace dialogs
} // namespace sv
