#include "ui/histogram_view.h"

#include <cstdio>

#include <FL/fl_draw.H>

HistogramView::HistogramView(int x, int y, int w, int h)
    : Fl_Widget(x, y, w, h), has_after(false) {
    for (int i = 0; i < 256; ++i) {
        before[i] = 0;
        after[i] = 0;
    }
}

void HistogramView::set_before(const int values[256]) {
    for (int i = 0; i < 256; ++i) {
        before[i] = values[i];
    }
    redraw();
}

void HistogramView::set_after(const int values[256]) {
    for (int i = 0; i < 256; ++i) {
        after[i] = values[i];
    }
    has_after = true;
    redraw();
}

void HistogramView::clear_after() {
    has_after = false;
    redraw();
}

void HistogramView::draw() {
    fl_color(fl_rgb_color(20, 20, 20));
    fl_rectf(x(), y(), w(), h());

    int pad_left = 34;
    int pad_right = 8;
    int pad_top = 22;
    int pad_bottom = 18;
    int plot_x = x() + pad_left;
    int plot_y = y() + pad_top;
    int plot_w = w() - pad_left - pad_right;
    int plot_h = h() - pad_top - pad_bottom;
    if (plot_w < 10 || plot_h < 10) return;

    int maximum = 1;
    for (int i = 0; i < 256; ++i) {
        if (before[i] > maximum) maximum = before[i];
        if (has_after && after[i] > maximum) maximum = after[i];
    }

    fl_color(fl_rgb_color(70, 70, 70));
    fl_line(plot_x, plot_y, plot_x, plot_y + plot_h);
    fl_line(plot_x, plot_y + plot_h, plot_x + plot_w, plot_y + plot_h);

    double scale = static_cast<double>(plot_h) / maximum;
    double bin_width = static_cast<double>(plot_w) / 256.0;

    fl_color(fl_rgb_color(150, 150, 150));
    for (int i = 0; i < 256; ++i) {
        if (before[i] <= 0) continue;
        int bar_h = static_cast<int>(before[i] * scale);
        if (bar_h < 1) bar_h = 1;
        int bx = plot_x + static_cast<int>(i * bin_width);
        int bw = static_cast<int>((i + 1) * bin_width) - static_cast<int>(i * bin_width);
        if (bw < 1) bw = 1;
        fl_rectf(bx, plot_y + plot_h - bar_h, bw, bar_h);
    }

    if (has_after) {
        fl_color(fl_rgb_color(80, 220, 80));
        fl_begin_line();
        for (int i = 0; i < 256; ++i) {
            double px = plot_x + (i + 0.5) * bin_width;
            double py = plot_y + plot_h - after[i] * scale;
            fl_vertex(px, py);
        }
        fl_end_line();
    }

    fl_color(fl_rgb_color(210, 210, 210));
    fl_font(FL_HELVETICA, 11);
    fl_draw("0", plot_x - 4, plot_y + plot_h + 12);
    fl_draw("255", plot_x + plot_w - 20, plot_y + plot_h + 12);

    char title[128];
    if (has_after) {
        snprintf(title, sizeof(title), "Гистограмма яркости: серый - до, зеленый - после");
    } else {
        snprintf(title, sizeof(title), "Гистограмма яркости: до");
    }
    fl_color(fl_rgb_color(230, 230, 230));
    fl_draw(title, x() + 8, y() + 14);
}
