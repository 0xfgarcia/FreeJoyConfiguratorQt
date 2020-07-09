#include "axescurvesplot.h"
#include "ui_axescurvesplot.h"
#include <QPainter>
#include <QMouseEvent>

#include <QDebug>
AxesCurvesPlot::AxesCurvesPlot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AxesCurvesPlot)
{
    ui->setupUi(this);
    setMouseTracking(true);

    points_count_ = 11;
    point_active_ = false;
    half_radius_ = radius_/2;

    uint tmp_range = abs(min_point_value) + abs(max_point_value);
    for (int i = 0; i < points_count_; ++i){
        AxesCurve_point* point = new AxesCurve_point;
        PointAdrList.append(point);
        PointAdrList[i]->color = point_inactive_color_;
        PointAdrList[i]->posX = 0;
        PointAdrList[i]->posY = 0;
        PointAdrList[i]->is_drag = false;
        PointAdrList[i]->current_value = ((tmp_range / (points_count_ - 1)) * i) - abs(min_point_value);    // hz
    }

}

AxesCurvesPlot::~AxesCurvesPlot()
{
    delete ui;
}
        ////////////////// half_radius_ - убрать                ///                 /// half_radius_

void AxesCurvesPlot::paintEvent(QPaintEvent *event)     // жирно, можно оптимизировать
{
    Q_UNUSED(event)
    QPainter painter;
    int tmp_x, tmp_y;
    half_radius_ = radius_/2;

    painter.begin(this);
    painter.setPen(Qt::lightGray);

//    if (resize_activated_ == true){
//        qDebug()<<"resize";
        //paint columns
        for (int i = 0; i < columns_count_ + 1; ++i){
            tmp_x = (i * column_width_) + offset_;
            painter.drawLine(tmp_x, offset_, tmp_x, height_ - offset_);
        }
        // paint rows
        for (int i = 0; i < rows_count_ + 1; ++i){
            tmp_y = (i * row_height_) + offset_;
            painter.drawLine(offset_, tmp_y, width_ - offset_, tmp_y);
        }
//        resize_activated_ = false;
//    }


    // Antialiasing         // можно светху но всё станет мазнёй
    painter.setRenderHint(QPainter::Antialiasing, true);
    // paint line       // можно в цикл paint rect, но лучше отдельно
    QPen pen;
    pen.setWidth(2);
    pen.setColor(point_inactive_color_);
    for (int i = 0; i < PointAdrList.size(); ++i)   // проверка на драг=труе + соседние
    {
        painter.setPen(pen);
        if (i < PointAdrList.size() - 1){
            painter.drawLine(PointAdrList[i]->posX  + half_radius_, PointAdrList[i]->posY  + half_radius_,
                             PointAdrList[i + 1]->posX  + half_radius_, PointAdrList[i + 1]->posY  + half_radius_);
        }
    }

    // coordinates for rect
    for (int i = 0; i < PointAdrList.size(); ++i) {
        PointAdrList[i]->area.setRect(PointAdrList[i]->posX, PointAdrList[i]->posY,
                                      radius_, radius_);
    }
    // paint rect
    painter.setPen(Qt::lightGray);
    for (int i = 0; i < PointAdrList.size(); ++i)   // проверка на драг=труе
    {
        painter.setBrush(PointAdrList[i]->color);
        painter.drawEllipse(PointAdrList[i]->area);
        painter.setBrush(point_move_color_);
        painter.drawEllipse(PointAdrList[i]->area.x() + half_radius_/2, PointAdrList[i]->area.y() + half_radius_/2,
                            half_radius_, half_radius_);
    }

    painter.end();
}


int AxesCurvesPlot::GetPointValue(int point_number)
{
    return PointAdrList[point_number]->current_value = CalcPointValue(PointAdrList[point_number]->posY);
}

int AxesCurvesPlot::GetPointCount()
{
    return points_count_;
}

void AxesCurvesPlot::SetPointValue(int value, int point_number)
{
    PointAdrList[point_number]->posY = CalcPointPos(value);
    PointAdrList[point_number]->current_value = value;
    update();
}

int AxesCurvesPlot::CalcPointValue(int current_pos)
{
    int value = 0;
    int half_height = (height_) / 2;    // отдельной переменной для оптимизации?
    current_pos += half_radius_;

    if (current_pos > half_height){
        float tmp_min = half_height / min_point_value;
        value = (current_pos - half_height) / tmp_min;
        //qDebug()<<"value ="<<value;
        return value;
    } else {
        float tmp_max = half_height / max_point_value;
        value = -(current_pos - half_height) / tmp_max;
        //qDebug()<<"value ="<<value;
        return value;
    }
    return value;
}

int AxesCurvesPlot::CalcPointPos(int value)     // хз, центр посередине, а дальше скейлится, в зависимости от значений min и max
{                                               // наверно лучше потом сделать центр в зависимости от значений min и max
    int pos = 0;
    int half_height = (height_) / 2;    // отдельной переменной для оптимизации?

    if (value < max_point_value - min_point_value){
        float tmp_min = half_height / min_point_value;
        pos = value * tmp_min + half_height;
    } else {
        float tmp_max = half_height / max_point_value;
        pos = value * tmp_max + half_height;
    }

    return pos - half_radius_;
}



void AxesCurvesPlot::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)

    int tmp_x;//, tmp_y;
    width_ = this->width();
    height_ = this->height();

    column_width_ = (width_ - offset_*2) / float(columns_count_);
    row_height_ = (height_ - offset_*2) / float(rows_count_);


    for (int i = 0; i < columns_count_ + 1; ++i){
        tmp_x = (i * column_width_) + offset_;
        PointAdrList[i]->posX = tmp_x - radius_/2;      // temp
    }
    for (int i = 0; i < PointAdrList.size(); ++i){
        //tmp_y = (i * row_height_) + offset_;
        PointAdrList[i]->posY = CalcPointPos(PointAdrList[i]->current_value);//tmp_y - radius_/2;      // temp
        //qDebug()<<"value"<<i<<PointAdrList[i]->current_value;
    }
//    for (int i = 0; i < rows_count_ + 1; ++i){
//        tmp_y = (i * row_height_) + offset_;
//        PointAdrList[i]->posY = tmp_y - radius_/2;      // temp
//    }

}

void AxesCurvesPlot::mouseMoveEvent(QMouseEvent *event)
{
    // оптимизировать, добавить проверку только по columns line + radius/2 ?
    for (int i = 0; i < PointAdrList.size(); ++i)
    {
        if (PointAdrList[i]->is_drag == false){
            if (PointAdrList[i]->area.contains(event->pos()))   // наверх
            {
                PointAdrList[i]->color = point_active_color_;
                point_active_ = true;
                update();
                break;  //?
            } else if (point_active_ == true){
                PointAdrList[i]->color = point_inactive_color_;
                update();
                //break;  //?
            }
        }
        else if (PointAdrList[i]->is_drag == true){     // if nah

            if (event->pos().y() < offset_ || event->pos().y() > this->height() - offset_) {
                return;
            }

            if (event->buttons() & Qt::LeftButton){
                PointAdrList[i]->posY = event->pos().y() - half_radius_;   // half_radius_ - убрать ненужные во всём коде
                PointAdrList[i]->color = point_move_color_;

                PointAdrList[i]->current_value = CalcPointValue(PointAdrList[i]->posY);
                update();
                //CalcPointValue(event->pos().y());
            }
            break;
        }
    }
    //update();
}

void AxesCurvesPlot::mousePressEvent(QMouseEvent *event)
{
    for (int i = 0; i < PointAdrList.size(); ++i){
        if (PointAdrList[i]->area.contains(event->pos())) {
            PointAdrList[i]->is_drag = true;
            break;
        }
    }
}
void AxesCurvesPlot::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    for (int i = 0; i < PointAdrList.size(); ++i){
        if(PointAdrList[i]->is_drag == true){
            PointAdrList[i]->is_drag = false;
            PointAdrList[i]->color = point_inactive_color_;
            break;
        }
    }
    update();
}
