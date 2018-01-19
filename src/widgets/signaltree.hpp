/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef WIDGETS_SIGNALTREE_HPP
#define WIDGETS_SIGNALTREE_HPP

#include <memory>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>

#include "src/session.hpp"
#include "src/channels/basechannel.hpp"

using std::map;
using std::shared_ptr;
using std::vector;

namespace sv {

namespace data {
class BaseSignal;
}

namespace devices {
class Device;
}

namespace widgets {

class SignalTree : public QTreeWidget
{
    Q_OBJECT

public:
	SignalTree(const Session &session,
		bool show_signals, bool selectable, bool multiselect,
		shared_ptr<devices::Device> selected_device,
		QWidget *parent = nullptr);

	vector<shared_ptr<channels::BaseChannel>> selected_channels();
	vector<shared_ptr<data::BaseSignal>> selected_signals();

private:
	void setup_ui();

	void add_device(shared_ptr<devices::HardwareDevice> device, bool expanded);
	void add_channel(shared_ptr<channels::BaseChannel> channel,
		QString channel_group_name, bool expanded, QTreeWidgetItem *parent);
	void add_signal(shared_ptr<data::BaseSignal> signal,
		QTreeWidgetItem *parent);

	vector<const QTreeWidgetItem *> checked_items();
	vector<const QTreeWidgetItem *> checked_items_recursiv(
		const QTreeWidgetItem * item);

	const Session &session_;
	shared_ptr<devices::Device> selected_device_;

	bool show_signals_;
	bool selectable_;
	bool multiselect_;

	map<shared_ptr<devices::Device>, QTreeWidgetItem *> device_map_;
	map<QString, QTreeWidgetItem *> channel_group_map_;
	map<shared_ptr<channels::BaseChannel>, QTreeWidgetItem *> channel_map_;

public Q_SLOTS:
	void on_device_added(shared_ptr<devices::HardwareDevice> device);
	void on_device_removed();

private Q_SLOTS:
	void on_channel_added(shared_ptr<channels::BaseChannel> channel);
	void on_channel_removed();
	void on_signal_added(shared_ptr<data::BaseSignal> signal);
	void on_signal_removed();

};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_SIGNALTREE_HPP

