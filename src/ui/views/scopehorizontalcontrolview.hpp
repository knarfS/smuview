/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_VIEWS_SCOPEHORIZONTALCONTROLVIEW_HPP
#define UI_VIEWS_SCOPEHORIZONTALCONTROLVIEW_HPP

#include <memory>

#include <QSettings>
#include <QUuid>

#include "src/ui/views/baseview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace devices {
class BaseDevice;
class Configurable;
}

namespace ui {

namespace datatypes {
class BoolCheckBox;
class RationalComboBox;
class UInt64ComboBox;
class UInt64Label;
class UInt64SpinBox;
}

namespace views {

class ScopeHorizontalControlView : public BaseView
{
	Q_OBJECT

public:
	ScopeHorizontalControlView(Session& session,
		std::shared_ptr<sv::devices::Configurable> configurable,
		QUuid uuid = QUuid(),
		QWidget* parent = nullptr);

	QString title() const override;

	void save_settings(QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device = nullptr) const override;
	void restore_settings(QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device = nullptr) override;
	static ScopeHorizontalControlView *init_from_settings(
		Session &session, QSettings &settings, QUuid uuid,
		shared_ptr<sv::devices::BaseDevice> origin_device);

private:
	shared_ptr<sv::devices::Configurable> configurable_;

	ui::datatypes::UInt64Label *samplerate_label_;
	ui::datatypes::RationalComboBox *timebase_box_;
	ui::datatypes::UInt64ComboBox *buffer_size_box_;
	ui::datatypes::BoolCheckBox *average_check_;
	ui::datatypes::UInt64ComboBox *average_count_box_;
	ui::datatypes::UInt64SpinBox *average_count_spin_;

	void setup_ui();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_SCOPEHORIZONTALCONTROLVIEW_HPP
