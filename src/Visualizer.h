#pragma once

#include <QWidget>
#include <QPainter>
#include "Arm.h"
#include "Search.h"

class Visualizer : public QWidget
{
Q_OBJECT

public:
    Visualizer(Arm& arm, Search& search, QWidget* parent = 0);

public slots:
    void heuristicOn(bool on);

private:
    void paintEvent(QPaintEvent*);

    Arm& arm;
    Search& search;
    bool draw_heuristic;
};
