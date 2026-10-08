#ifndef HISTOGRAM_VIEW_H
#define HISTOGRAM_VIEW_H

#include <FL/Fl_Widget.H>

class HistogramView : public Fl_Widget {
public:
    HistogramView(int x, int y, int w, int h);

    void set_before(const int values[256]);
    void set_after(const int values[256]);
    void clear_after();
    void draw() FL_OVERRIDE;

private:
    int before[256];
    int after[256];
    bool has_after;
};

#endif
