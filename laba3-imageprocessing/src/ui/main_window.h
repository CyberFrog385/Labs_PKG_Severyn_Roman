#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <FL/Fl_Double_Window.H>

#include "core/image_buffer.h"
#include "ui/histogram_view.h"
#include "ui/image_view.h"
#include "ui/worker.h"

class Fl_Menu_Bar;
class Fl_Value_Slider;
class Fl_Progress;
class Fl_Box;
class Fl_Button;

class MainWindow : public Fl_Double_Window {
public:
    MainWindow();

    void dispatch(int op);
    void tick();
    void open_path(const char* path);

    Worker worker;

private:
    void build_ui();
    void open_image();
    void save_result();
    void start_operation(int op);
    void apply_result();
    void reset_to_original();
    void refresh_histograms();
    void update_views();
    void set_status(const char* text);

    ImageBuffer pristine;
    ImageBuffer current;
    ImageBuffer result;

    ImageView* before_view;
    ImageView* after_view;
    HistogramView* histogram_view;
    Fl_Menu_Bar* menu_bar;
    Fl_Value_Slider* contrast_min_slider;
    Fl_Value_Slider* contrast_max_slider;
    Fl_Value_Slider* gamma_slider;
    Fl_Value_Slider* constant_slider;
    Fl_Value_Slider* factor_slider;
    Fl_Value_Slider* clahe_clip_slider;
    Fl_Value_Slider* clahe_tiles_slider;
    Fl_Progress* progress_bar;
    Fl_Box* status_box;
};

#endif
