/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDebug>
#include <QFont>
#include <QFontDatabase>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>

#include "monofontdisplay.hpp"
#include "src/ui/widgets/valuedisplay.hpp"

namespace sv {
namespace ui {
namespace widgets {

MonoFontDisplay::MonoFontDisplay(
		int digits, int decimal_places, const bool auto_range,
		const QString unit, const QString unit_suffix, const QString extra_text,
		const bool small, QWidget *parent) :
	ValueDisplay(digits, decimal_places, auto_range, unit, unit_suffix,
		extra_text, small, parent)
{
	setup_ui();
	reset_value();
}

void MonoFontDisplay::setup_ui()
{
	// Get standard font sizes
	QFont monospace_font("Monospace"); // = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	monospace_font.setStyleHint(QFont::TypeWriter);
	int monospace_font_size = monospace_font.pointSize();
	int std_font_size = QFont().pointSize();

	if (!small_) {
		value_font_size_ = monospace_font_size + 12; // 22
		unit_font_size_ = std_font_size + 8; // 18
		extra_font_size_ = std_font_size; // 10
	}
	else {
		value_font_size_ = monospace_font_size + 4; // 14
		unit_font_size_ = std_font_size; // 10
		extra_font_size_ = std_font_size - 3; // 7
	}

	//this->setFrameShape(QFrame::Box);
	QSizePolicy layout_size_policy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	layout_size_policy.setHorizontalStretch(0);
	layout_size_policy.setVerticalStretch(0);
	this->setSizePolicy(layout_size_policy);

	QHBoxLayout *layout = new QHBoxLayout();

	value_label_ = new QLabel();
	QFont value_font(monospace_font);
	value_font.setPointSize(value_font_size_);
	if (!small_) {
		value_font.setBold(true);
		value_font.setWeight(QFont::Bold);
	}
	value_label_->setFont(value_font);
	value_label_->setAlignment(Qt::AlignRight | Qt::AlignBottom);
	value_label_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	//value_label_->setFrameShape(QFrame::Box);
	layout->addWidget(value_label_);
	layout->setAlignment(unit_label_, Qt::AlignBottom);

	QVBoxLayout *text_layout = new QVBoxLayout();

	// Extra text (small)
	extra_label_ = new QLabel();
	QFont extra_font;
	extra_font.setPointSize(extra_font_size_);
	extra_label_->setFont(extra_font);
	extra_label_->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
	extra_label_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	//extra_label_->setFrameShape(QFrame::Box);
	text_layout->addWidget(extra_label_);
	text_layout->setAlignment(extra_label_, Qt::AlignHCenter);

	// Unit
	unit_label_ = new QLabel();
	QFont unit_font;
	unit_font.setPointSize(unit_font_size_);
	unit_label_->setFont(unit_font);
	unit_label_->setAlignment(Qt::AlignRight | Qt::AlignBottom);
	unit_label_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	//unit_label_->setFrameShape(QFrame::Box);
	text_layout->addWidget(unit_label_);
	text_layout->setAlignment(unit_label_, Qt::AlignHCenter | Qt::AlignBottom);

	layout->addLayout(text_layout);
	this->setLayout(layout);
}

void MonoFontDisplay::update_value_widget_dimensions()
{
	QString str("");
	size_t str_len = digits_ + 2; // digits + decimal point + sign
	for (size_t i=0; i<str_len; ++i) {
		str += "-";
	}
	QFontMetrics metrics = value_label_->fontMetrics();
	value_label_->setFixedWidth(metrics.width(str));
}

void MonoFontDisplay::update_extra_widget_dimensions()
{
	if (extra_text_.isEmpty()) {
		extra_label_->hide();
	}
	else {
		extra_label_->show();
	}
}

void MonoFontDisplay::update_unit_widget_dimensions()
{
	QString str;
	if (auto_range_) {
		// 'm' is the widest character for non monospace fonts
		str.append("m");
	}
	str.append(unit_);
	if (!unit_suffix_.isEmpty()) {
		str.append(" ").append(unit_suffix_);
	}
	QFontMetrics metrics = unit_label_->fontMetrics();
	unit_label_->setFixedWidth(metrics.width(str));
}

void MonoFontDisplay::show_value(const QString &value)
{
	value_label_->setText(value);
}

void MonoFontDisplay::show_extra_text(const QString &extra_text)
{
	extra_label_->setText(extra_text);
}

void MonoFontDisplay::show_unit(const QString &unit)
{
	unit_label_->setText(unit);
}

} // namespace widgets
} // namespace ui
} // namespace sv
