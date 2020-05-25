/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_VIEWS_SOURCESINKCONTROLVIEW_HPP
#define UI_VIEWS_SOURCESINKCONTROLVIEW_HPP

#include <memory>

#include <QSettings>
#include <QUuid>

#include "src/devices/deviceutil.hpp"
#include "src/ui/views/baseview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace devices {
class Configurable;
}

namespace ui {

namespace datatypes {
class BoolButton;
class BoolLed;
class DoubleControl;
class StringComboBox;
class StringLed;
class ThresholdControl;
}

namespace views {

class SourceSinkControlView : public BaseView
{
	Q_OBJECT

public:
	SourceSinkControlView(Session& session,
		std::shared_ptr<sv::devices::Configurable> configurable,
		QUuid uuid = QUuid(),
		QWidget* parent = nullptr);

	QString title() const override;

	// TODO: scope
	void save_settings(QSettings &settings) const override;
	void restore_settings(QSettings &settings) override;

private:
	shared_ptr<sv::devices::Configurable> configurable_;

	ui::datatypes::StringLed *cc_led_;
	ui::datatypes::StringLed *cv_led_;
	ui::datatypes::BoolLed *ovp_led_;
	ui::datatypes::BoolLed *ocp_led_;
	ui::datatypes::BoolLed *otp_led_;
	ui::datatypes::BoolLed *uvc_led_;
	ui::datatypes::BoolButton *enable_button_;
	ui::datatypes::StringComboBox *regulation_box_;
	ui::datatypes::DoubleControl *voltage_control_;
	ui::datatypes::DoubleControl *current_control_;
	ui::datatypes::ThresholdControl *ovp_control_;
	ui::datatypes::ThresholdControl *ocp_control_;
	ui::datatypes::ThresholdControl *uvc_control_;

	void setup_ui();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_SOURCESINKCONTROLVIEW_HPP
