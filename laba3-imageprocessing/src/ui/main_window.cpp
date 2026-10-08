#include "ui/main_window.h"

#include <cstdint>
#include <cstdio>
#include <string>

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/fl_ask.H>

#include "core/clahe.h"
#include "core/image_io.h"
#include "core/processing.h"

namespace {

enum Operation {
    OP_OPEN = 1,
    OP_SAVE,
    OP_QUIT,
    OP_CONTRAST_AUTO,
    OP_CONTRAST_MANUAL,
    OP_EQUALIZE_RGB,
    OP_EQUALIZE_HSV,
    OP_CLAHE,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NEG,
    OP_GAMMA,
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_APPLY,
    OP_RESET,
    OP_REFRESH,
    OP_ABOUT
};

const int WINDOW_W = 1280;
const int WINDOW_H = 772;
const int PANEL_Y = 32;
const int PANEL_H = 450;
const int PANEL_W = 628;
const int HIST_Y = 488;
const int HIST_H = 168;
const int SLIDER_Y = 674;
const int PROGRESS_Y = 704;
const int BOTTOM_Y = 734;

void menu_callback(Fl_Widget* widget, void* data);

Fl_Menu_Item menu_items[] = {
    {"Файл", 0, 0, 0, FL_SUBMENU},
        {"Открыть...", FL_COMMAND + 'o', menu_callback, (void*)(intptr_t)OP_OPEN, 0},
        {"Сохранить результат (PNG)...", FL_COMMAND + 's', menu_callback, (void*)(intptr_t)OP_SAVE, FL_MENU_DIVIDER},
        {"Выход", FL_COMMAND + 'q', menu_callback, (void*)(intptr_t)OP_QUIT, 0},
    {"Контрастирование", 0, 0, 0, FL_SUBMENU},
        {"Линейное (авто)", 0, menu_callback, (void*)(intptr_t)OP_CONTRAST_AUTO, 0},
        {"Линейное (ручное, слайдеры min/max)", 0, menu_callback, (void*)(intptr_t)OP_CONTRAST_MANUAL, 0},
    {"Гистограмма", 0, 0, 0, FL_SUBMENU},
        {"Эквализация (по каналам RGB)", 0, menu_callback, (void*)(intptr_t)OP_EQUALIZE_RGB, 0},
        {"Эквализация (яркость HSV)", 0, menu_callback, (void*)(intptr_t)OP_EQUALIZE_HSV, 0},
        {"CLAHE (яркость HSV)", 0, menu_callback, (void*)(intptr_t)OP_CLAHE, 0},
        {"Обновить гистограммы", 0, menu_callback, (void*)(intptr_t)OP_REFRESH, FL_MENU_DIVIDER},
    {"Поэлементные", 0, 0, 0, FL_SUBMENU},
        {"Сложить с константой", 0, menu_callback, (void*)(intptr_t)OP_ADD, 0},
        {"Вычесть константу", 0, menu_callback, (void*)(intptr_t)OP_SUB, 0},
        {"Умножить на константу", 0, menu_callback, (void*)(intptr_t)OP_MUL, 0},
        {"Разделить на константу", 0, menu_callback, (void*)(intptr_t)OP_DIV, 0},
        {"Негатив", 0, menu_callback, (void*)(intptr_t)OP_NEG, 0},
        {"Гамма-коррекция", 0, menu_callback, (void*)(intptr_t)OP_GAMMA, 0},
        {"AND с константой", 0, menu_callback, (void*)(intptr_t)OP_AND, 0},
        {"OR с константой", 0, menu_callback, (void*)(intptr_t)OP_OR, 0},
        {"XOR с константой", 0, menu_callback, (void*)(intptr_t)OP_XOR, 0},
    {"Правка", 0, 0, 0, FL_SUBMENU},
        {"Применить результат к исходному", 0, menu_callback, (void*)(intptr_t)OP_APPLY, 0},
        {"Сброс к исходному", 0, menu_callback, (void*)(intptr_t)OP_RESET, FL_MENU_DIVIDER},
    {"Справка", 0, 0, 0, FL_SUBMENU},
        {"О программе", 0, menu_callback, (void*)(intptr_t)OP_ABOUT, 0},
    {0, 0, 0, 0, 0}
};

void menu_callback(Fl_Widget* widget, void* data) {
    Fl_Menu_Bar* bar = static_cast<Fl_Menu_Bar*>(widget);
    MainWindow* self = static_cast<MainWindow*>(bar->user_data());
    int op = static_cast<int>(reinterpret_cast<intptr_t>(data));
    self->dispatch(op);
}

void apply_callback(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->dispatch(OP_APPLY);
}

void reset_callback(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->dispatch(OP_RESET);
}

void tick_callback(void* data) {
    MainWindow* self = static_cast<MainWindow*>(data);
    self->tick();
    Fl::add_timeout(0.05, tick_callback, data);
}

Fl_Value_Slider* make_slider(int x, int y, int w, const char* label,
                             double low, double high, double initial,
                             double step_value, int precision) {
    Fl_Value_Slider* slider = new Fl_Value_Slider(x, y, w, 20);
    slider->type(FL_HORIZONTAL);
    slider->bounds(low, high);
    slider->step(step_value);
    slider->precision(precision);
    slider->value(initial);
    slider->label(label);
    slider->align(FL_ALIGN_TOP);
    slider->labelsize(11);
    slider->labelcolor(fl_rgb_color(220, 220, 220));
    return slider;
}

}

MainWindow::MainWindow()
    : Fl_Double_Window(WINDOW_W, WINDOW_H, "Лабораторная 3 - Обработка изображений"),
      before_view(NULL),
      after_view(NULL),
      histogram_view(NULL),
      menu_bar(NULL),
      contrast_min_slider(NULL),
      contrast_max_slider(NULL),
      gamma_slider(NULL),
      constant_slider(NULL),
      factor_slider(NULL),
      clahe_clip_slider(NULL),
      clahe_tiles_slider(NULL),
      progress_bar(NULL),
      status_box(NULL) {
    build_ui();
    color(fl_rgb_color(50, 50, 50));
    Fl::add_timeout(0.05, tick_callback, this);
    end();
}

void MainWindow::build_ui() {
    menu_bar = new Fl_Menu_Bar(0, 0, WINDOW_W, 26);
    menu_bar->menu(menu_items);
    menu_bar->user_data(this);
    menu_bar->labelsize(13);

    before_view = new ImageView(8, PANEL_Y, PANEL_W, PANEL_H, "");
    after_view = new ImageView(644, PANEL_Y, PANEL_W, PANEL_H, "");
    before_view->set_caption("До (исходное)");
    after_view->set_caption("После (результат)");

    histogram_view = new HistogramView(8, HIST_Y, WINDOW_W - 16, HIST_H);

    const char* labels[7] = {
        "Контраст min", "Контраст max", "Гамма",
        "Константа", "Множитель", "CLAHE лимит", "CLAHE тайлы"
    };
    int column_w = 175;
    int column_gap = 6;
    int start_x = 8;
    contrast_min_slider = make_slider(start_x + 0 * (column_w + column_gap), SLIDER_Y, column_w,
                                      labels[0], 0, 255, 0, 1, 0);
    contrast_max_slider = make_slider(start_x + 1 * (column_w + column_gap), SLIDER_Y, column_w,
                                      labels[1], 0, 255, 255, 1, 0);
    gamma_slider = make_slider(start_x + 2 * (column_w + column_gap), SLIDER_Y, column_w,
                               labels[2], 0.10, 3.00, 1.00, 0.05, 2);
    constant_slider = make_slider(start_x + 3 * (column_w + column_gap), SLIDER_Y, column_w,
                                  labels[3], -255, 255, 30, 1, 0);
    factor_slider = make_slider(start_x + 4 * (column_w + column_gap), SLIDER_Y, column_w,
                                labels[4], 0.10, 3.00, 1.30, 0.05, 2);
    clahe_clip_slider = make_slider(start_x + 5 * (column_w + column_gap), SLIDER_Y, column_w,
                                    labels[5], 0.50, 10.00, 2.00, 0.10, 2);
    clahe_tiles_slider = make_slider(start_x + 6 * (column_w + column_gap), SLIDER_Y, column_w,
                                     labels[6], 2, 16, 8, 1, 0);

    progress_bar = new Fl_Progress(8, PROGRESS_Y, 700, 22);
    progress_bar->minimum(0.0);
    progress_bar->maximum(1.0);
    progress_bar->value(0.0);
    progress_bar->label("Обработка");
    progress_bar->labelsize(11);

    status_box = new Fl_Box(716, PROGRESS_Y, WINDOW_W - 724, 22, "");
    status_box->box(FL_FLAT_BOX);
    status_box->color(fl_rgb_color(35, 35, 35));
    status_box->labelcolor(fl_rgb_color(210, 210, 210));
    status_box->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    status_box->labelsize(12);
    status_box->copy_label("Откройте изображение: Файл -> Открыть");

    Fl_Button* apply_button = new Fl_Button(8, BOTTOM_Y, 250, 28, "Применить результат");
    apply_button->callback(apply_callback, this);
    apply_button->labelsize(12);

    Fl_Button* reset_button = new Fl_Button(266, BOTTOM_Y, 170, 28, "Сброс");
    reset_button->callback(reset_callback, this);
    reset_button->labelsize(12);

    Fl_Box* hint = new Fl_Box(444, BOTTOM_Y, WINDOW_W - 452, 28,
                              "Результат операций появляется справа; применяйте его к исходному для последовательных операций");
    hint->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    hint->labelsize(11);
    hint->labelcolor(fl_rgb_color(170, 170, 170));
}

void MainWindow::set_status(const char* text) {
    status_box->copy_label(text);
    status_box->redraw();
}

void MainWindow::update_views() {
    if (current.empty()) {
        before_view->set_image(NULL);
    } else {
        before_view->set_image(&current);
    }
    if (result.empty()) {
        after_view->set_image(NULL);
    } else {
        after_view->set_image(&result);
    }
}

void MainWindow::refresh_histograms() {
    int histogram[256];
    if (current.empty()) {
        for (int i = 0; i < 256; ++i) histogram[i] = 0;
        histogram_view->set_before(histogram);
        histogram_view->clear_after();
        return;
    }
    compute_histogram(current, -1, histogram);
    histogram_view->set_before(histogram);
    if (result.empty()) {
        histogram_view->clear_after();
    } else {
        compute_histogram(result, -1, histogram);
        histogram_view->set_after(histogram);
    }
}

void MainWindow::open_image() {
    const char* chosen = fl_file_chooser("Открыть изображение",
                                         "Images\t*.{png,jpg,jpeg,bmp,gif,tif,tiff,pnm}\tAll Files\t*",
                                         "");
    if (!chosen) return;
    open_path(chosen);
}

void MainWindow::open_path(const char* path) {
    std::string error;
    ImageBuffer loaded = load_image(path, &error);
    if (loaded.empty()) {
        fl_alert("%s", error.c_str());
        return;
    }
    pristine = loaded;
    current = loaded;
    result = ImageBuffer();
    update_views();
    refresh_histograms();
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s: %dx%d, %s, %.1f Мп",
             path,
             current.width(),
             current.height(),
             current.channels() == 1 ? "серое" : "RGB",
             current.width() * current.height() / 1000000.0);
    set_status(buffer);
}

void MainWindow::save_result() {
    if (result.empty()) {
        fl_message("Нет результата для сохранения. Сначала выполните операцию.");
        return;
    }
    const char* chosen = fl_file_chooser("Сохранить результат", "PNG\t*.png", "result.png");
    if (!chosen) return;
    std::string path = chosen;
    std::string error;
    if (!save_png(result, path, &error)) {
        fl_alert("%s", error.c_str());
        return;
    }
    set_status(("Сохранено: " + path).c_str());
}

void MainWindow::start_operation(int op) {
    if (worker.busy()) {
        set_status("Идет обработка, дождитесь завершения");
        return;
    }
    if (current.empty()) {
        set_status("Сначала откройте изображение");
        return;
    }
    int in_min = static_cast<int>(contrast_min_slider->value());
    int in_max = static_cast<int>(contrast_max_slider->value());
    double gamma_value = gamma_slider->value();
    int constant = static_cast<int>(constant_slider->value());
    double factor_value = factor_slider->value();
    double clip_value = clahe_clip_slider->value();
    int tiles_value = static_cast<int>(clahe_tiles_slider->value());
    progress_bar->value(0.0);
    set_status("Обработка...");

    worker.start(
        [this, op, in_min, in_max, gamma_value, constant, factor_value, clip_value, tiles_value](Progress* progress) -> ImageBuffer {
            switch (op) {
                case OP_CONTRAST_AUTO:
                    return linear_contrast_auto(current, progress);
                case OP_CONTRAST_MANUAL:
                    return linear_contrast_manual(current, in_min, in_max, progress);
                case OP_EQUALIZE_RGB:
                    return equalize_rgb(current, progress);
                case OP_EQUALIZE_HSV:
                    return equalize_hsv(current, progress);
                case OP_CLAHE:
                    return clahe(current, tiles_value, clip_value, progress);
                case OP_ADD:
                    return pixel_add_constant(current, constant, progress);
                case OP_SUB:
                    return pixel_subtract_constant(current, constant, progress);
                case OP_MUL:
                    return pixel_multiply_constant(current, factor_value, progress);
                case OP_DIV:
                    return pixel_divide_constant(current, factor_value, progress);
                case OP_GAMMA:
                    return pixel_gamma(current, gamma_value, progress);
                case OP_NEG:
                    return pixel_negative(current, progress);
                case OP_AND:
                    return pixel_and_constant(current, constant, progress);
                case OP_OR:
                    return pixel_or_constant(current, constant, progress);
                case OP_XOR:
                    return pixel_xor_constant(current, constant, progress);
                default:
                    return ImageBuffer();
            }
        },
        [this](const ImageBuffer& output) {
            result = output;
            update_views();
            refresh_histograms();
            progress_bar->value(1.0);
            double before_stats[4];
            double after_stats[4];
            compute_statistics(current, -1, before_stats);
            compute_statistics(result, -1, after_stats);
            char buffer[256];
            snprintf(buffer, sizeof(buffer),
                     "Готово: среднее %.1f -> %.1f, сигма %.1f -> %.1f, размах %.0f..%.0f -> %.0f..%.0f",
                     before_stats[2], after_stats[2],
                     before_stats[3], after_stats[3],
                     before_stats[0], before_stats[1],
                     after_stats[0], after_stats[1]);
            set_status(buffer);
        });
}

void MainWindow::apply_result() {
    if (worker.busy()) {
        set_status("Идет обработка, дождитесь завершения");
        return;
    }
    if (result.empty()) {
        set_status("Нет результата для применения");
        return;
    }
    current = result;
    result = ImageBuffer();
    update_views();
    refresh_histograms();
    set_status("Результат применен к исходному");
}

void MainWindow::reset_to_original() {
    if (worker.busy()) {
        set_status("Идет обработка, дождитесь завершения");
        return;
    }
    if (pristine.empty()) {
        set_status("Изображение не загружено");
        return;
    }
    current = pristine;
    result = ImageBuffer();
    update_views();
    refresh_histograms();
    set_status("Сброшено к исходному изображению");
}

void MainWindow::dispatch(int op) {
    switch (op) {
        case OP_OPEN:
            open_image();
            break;
        case OP_SAVE:
            save_result();
            break;
        case OP_QUIT:
            hide();
            break;
        case OP_APPLY:
            apply_result();
            break;
        case OP_RESET:
            reset_to_original();
            break;
        case OP_REFRESH:
            refresh_histograms();
            set_status("Гистограммы обновлены");
            break;
        case OP_ABOUT:
            fl_message("Лабораторная работа 3, вариант 1\n"
                       "Гистограмма и эквализация + линейное контрастирование\n"
                       "Поэлементные операции + линейное контрастирование\n"
                       "Интерфейс: FLTK 1.4.5");
            break;
        default:
            start_operation(op);
            break;
    }
}

void MainWindow::tick() {
    worker.poll();
    if (worker.busy()) {
        int total = worker.progress.total.load();
        int value = worker.progress.value.load();
        if (total > 0) {
            double fraction = static_cast<double>(value) / total;
            if (fraction > 1.0) fraction = 1.0;
            progress_bar->value(fraction);
        }
    }
}
