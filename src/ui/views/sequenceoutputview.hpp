/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_VIEWS_SEQUENCEOUTPUTVIEW_HPP
#define UI_VIEWS_SEQUENCEOUTPUTVIEW_HPP

#include <memory>

#include <QAction>
#include <QCheckBox>
#include <QLocale>
#include <QSpinBox>
#include <QString>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QTimer>
#include <QToolBar>
#include <QVariant>

#include "src/ui/views/baseview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace data {
namespace properties {
class DoubleProperty;
}
}

namespace ui {
namespace views {

class DoubleSpinBoxDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	DoubleSpinBoxDelegate(double min, double max, double step, int decimals,
		QObject *parent = nullptr);

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const QModelIndex &index) const override;
	QString displayText(const QVariant &value,
		const QLocale &locale) const override;
	void setEditorData(QWidget *editor,
		const QModelIndex &index) const override;
	void setModelData(QWidget *editor, QAbstractItemModel *model,
		const QModelIndex &index) const override;
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
		const QModelIndex &index) const override;

private:
	double min_;
	double max_;
	double step_;
	int decimals_;

};

class SequenceOutputView : public BaseView
{
	Q_OBJECT

public:
	SequenceOutputView(Session& session,
		shared_ptr<sv::data::properties::DoubleProperty> property,
		QWidget* parent = nullptr);
	~SequenceOutputView();

	QString title() const override;

private:
	shared_ptr<sv::data::properties::DoubleProperty> property_;
	QAction *const action_run_;
	QAction *const action_add_row_;
	QAction *const action_delete_row_;
	QAction *const action_delete_all_;
	QAction *const action_load_from_file_;
	QAction *const action_generate_waveform_;
	QToolBar *toolbar_;
	QTimer *timer_;
	QCheckBox *repeat_infinite_box_;
	QSpinBox *repeat_count_box_;
	QTableWidget *sequence_table_;
	int sequence_pos_;
	int sequence_reperat_count_;

	void setup_ui();
	void setup_toolbar();
	void start_timer();
	void stop_timer();
	void insert_row(int row, double value, double delay);
	QStringList parse_csv_line(QString line);

private Q_SLOTS:
	void on_timer_update();
	void on_repeat_infinite_changed();
	void on_action_run_triggered();
	void on_action_add_row();
	void on_action_delete_row();
	void on_action_delete_all();
	void on_action_load_from_file_triggered();
	void on_action_generate_waveform_triggered();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_SEQUENCEOUTPUTVIEW_HPP
