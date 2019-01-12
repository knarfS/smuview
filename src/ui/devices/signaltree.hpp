/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DEVICES_SIGNALTREE_HPP
#define UI_DEVICES_SIGNALTREE_HPP

#include <memory>

#include <QPoint>
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
class BaseDevice;
}

namespace ui {
namespace devices {

class SignalTree : public QTreeWidget
{
	Q_OBJECT

public:
	SignalTree(const Session &session,
		bool show_signals, bool selectable, bool multiselect,
		shared_ptr<sv::devices::BaseDevice> selected_device,
		QWidget *parent = nullptr);

	vector<shared_ptr<channels::BaseChannel>> selected_channels();
	vector<shared_ptr<sv::data::BaseSignal>> selected_signals();
	void enable_context_menu(bool enable);
	void enable_tool_bar(bool enable);

private:
	void setup_ui();

	void add_device(shared_ptr<sv::devices::BaseDevice> device, bool expanded);
	void add_channel(shared_ptr<channels::BaseChannel> channel,
		QString channel_group_name, bool expanded, QTreeWidgetItem *parent);
	void add_signal(shared_ptr<sv::data::BaseSignal> signal,
		QTreeWidgetItem *parent);

	vector<const QTreeWidgetItem *> checked_items();
	vector<const QTreeWidgetItem *> checked_items_recursiv(
		const QTreeWidgetItem * item);

	QTreeWidgetItem *find_item(QTreeWidgetItem *parent, QString name);

	const Session &session_;
	shared_ptr<sv::devices::BaseDevice> selected_device_;

	bool show_signals_;
	bool selectable_;
	bool multiselect_;

	map<shared_ptr<sv::devices::BaseDevice>, QTreeWidgetItem *> device_map_;
	map<shared_ptr<channels::BaseChannel>, QTreeWidgetItem *> channel_map_;

public Q_SLOTS:
	void on_device_added(shared_ptr<sv::devices::BaseDevice> device);
	void on_device_removed();

private Q_SLOTS:
	void on_channel_added(shared_ptr<channels::BaseChannel> channel);
	void on_channel_removed();
	void on_signal_added(shared_ptr<sv::data::BaseSignal> signal);
	void on_signal_removed();
	void on_context_menu(const QPoint &);
	void on_add_user_channel();

};

} // namespace devices
} // namespace ui
} // namespace sv

#endif // UI_DEVICES_SIGNALTREE_HPP
