/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>
#include <mutex>
#include <string>

#include <QAction>
#include <QDebug>
#include <QScrollBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolBar>
#include <QVBoxLayout>

#include "dataview.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/ui/dialogs/selectsignaldialog.hpp"

using std::dynamic_pointer_cast;

namespace sv {
namespace ui {
namespace views {

DataView::DataView(Session &session,
		shared_ptr<sv::data::AnalogTimeSignal> signal,
		QWidget *parent) :
	BaseView(session, parent),
	auto_scroll_(true),
	action_auto_scroll_(new QAction(this)),
	action_add_signal_(new QAction(this))
{
	id_ = "data:" + signal->name();

	setup_ui();
	setup_toolbar();
	add_signal(signal);
}

QString DataView::title() const
{
	return signals_[0]->display_name() + " " + tr("Data");
}

void DataView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	data_table_ = new QTableWidget();
	data_table_->setColumnCount(1);
	QTableWidgetItem *time_header_item = new QTableWidgetItem(tr("Time [s]"));
	time_header_item->setTextAlignment(Qt::AlignVCenter);
	data_table_->setHorizontalHeaderItem(0, time_header_item);
	data_table_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	layout->addWidget(data_table_);

	this->central_widget_->setLayout(layout);
}

void DataView::setup_toolbar()
{
	action_auto_scroll_->setText(tr("Auto scroll"));
	action_auto_scroll_->setIcon(
		QIcon::fromTheme("go-bottom",
		QIcon(":/icons/go-bottom.png")));
	action_auto_scroll_->setCheckable(true);
	action_auto_scroll_->setChecked(auto_scroll_);
	connect(action_auto_scroll_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_auto_scroll_triggered()));

	action_add_signal_->setText(tr("Add signal"));
	action_add_signal_->setIcon(
		QIcon::fromTheme("office-chart-line",
		QIcon(":/icons/office-chart-line.png")));
	connect(action_add_signal_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_signal_triggered()));

	toolbar_ = new QToolBar("Data View Toolbar");
	toolbar_->addAction(action_auto_scroll_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_add_signal_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void DataView::add_signal(shared_ptr<sv::data::AnalogTimeSignal> signal)
{
	signals_.push_back(signal);
	next_signal_pos_.push_back(0);
	last_timestamp_.push_back(nullptr);
	size_t pos = signals_.size();

	QTableWidgetItem *value_header_item = new QTableWidgetItem(
		signal->display_name());
	value_header_item->setTextAlignment(Qt::AlignVCenter);
	data_table_->insertColumn(pos);
	data_table_->setHorizontalHeaderItem(pos, value_header_item);

	this->populate_table();
	connect(signal.get(), SIGNAL(sample_appended()),
		this, SLOT(populate_table()));
}

void DataView::populate_table()
{
	std::unique_lock<std::mutex> lock(populate_mutex_, std::try_to_lock);
	if (!lock.owns_lock())
		return;

	for (size_t i=0; i<signals_.size(); ++i) {
		size_t signal_size = signals_[i]->sample_count();
		while (next_signal_pos_[i] < signal_size) {
			auto sample = signals_[i]->get_sample(next_signal_pos_[i], true);
			int row_count  = data_table_->rowCount();

			int last_row  = -1;
			if (last_timestamp_[i])
				last_row = data_table_->row(last_timestamp_[i]);

			bool new_row = false;
			if (row_count <= last_row+1) {
				data_table_->insertRow(last_row+1);
				new_row = true;
			}
			else {
				// Find position of new sample
				while (last_row+1 < row_count) {
					auto item = data_table_->item(last_row+1, 0);
					double timestamp = item->data(0).toDouble();
					if (timestamp > sample.first) {
						data_table_->insertRow(last_row+1);
						new_row = true;
						break;
					}
					if (timestamp == sample.first) {
						last_timestamp_[i] = item;
						new_row = false;
						break;
					}
					if (row_count == last_row+2) {
						++last_row;
						data_table_->insertRow(last_row+1);
						new_row = true;
						break;
					}
					last_row++;
				}
			}

			if (new_row) {
				QTableWidgetItem *time_item = new QTableWidgetItem(sample.first);
				time_item->setData(0, QVariant(sample.first));
				data_table_->setItem(last_row+1, 0, time_item);
				last_timestamp_[i] = time_item;
			}
			QTableWidgetItem *value_item = new QTableWidgetItem(sample.second);
			value_item->setData(0, QVariant(sample.second));
			data_table_->setItem(last_row+1, i+1, value_item);

			++next_signal_pos_[i];
		}
	}
	if (auto_scroll_)
		data_table_->scrollToBottom();
}

void DataView::on_action_auto_scroll_triggered()
{
	auto_scroll_ = !auto_scroll_;
	action_auto_scroll_->setChecked(auto_scroll_);
}

void DataView::on_action_add_signal_triggered()
{
	shared_ptr<sv::devices::BaseDevice> selected_device;
	if (signals_[0])
		selected_device = signals_[0]->parent_channel()->parent_device();

	ui::dialogs::SelectSignalDialog dlg(session(), selected_device);
	if (!dlg.exec())
		return;

	for (const auto &signal : dlg.signals()) {
		add_signal(dynamic_pointer_cast<sv::data::AnalogTimeSignal>(signal));
	}
}

} // namespace views
} // namespace ui
} // namespace sv

