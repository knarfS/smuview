/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_TABS_SMUSCRIPTTAB_HPP
#define UI_TABS_SMUSCRIPTTAB_HPP

#include <QWidget>

#include "src/ui/tabs/basetab.hpp"

using std::string;

namespace sv {

class Session;

namespace ui {

namespace views {
class SmuScriptOutputView;
class SmuScriptView;
}

namespace tabs {

class SmuScriptTab : public BaseTab
{
	Q_OBJECT

private:

public:
	SmuScriptTab(Session &session, string script_file_name,
		QWidget *parent = nullptr);

	string tab_id() override;
	QString tab_title() override;
	bool request_close() override;

private:
	void setup_ui();
	void connect_signals();

	static unsigned int smuscript_tab_counter;

	string tab_id_;
	string script_file_name_;
	views::SmuScriptView *smu_script_view_;
	views::SmuScriptOutputView *smu_script_output_view_;

private Q_SLOTS:
	void on_file_name_changed(const QString &file_name);
	void on_file_save_state_changed(bool is_unsaved);
	void on_script_started();
	void on_script_finished();

};

} // namespace tabs
} // namespace ui
} // namespace sv

#endif // UI_TABS_SMUSCRIPTTAB_HPP
