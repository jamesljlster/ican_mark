#ifndef MARKAREA_H
#define MARKAREA_H

#include <map>
#include <string>

#include <QEvent>
#include <QImage>
#include <QObject>
#include <QWidget>

#include <mark_action.hpp>

class MarkArea : public QWidget
{
    Q_OBJECT
   public:
    explicit MarkArea(QWidget* parent = nullptr);

   signals:

   protected:
    bool event(QEvent* event);
    void paintEvent(QPaintEvent* paintEvent);

    ical_mark::RBoxMark testAction;

    QImage bgImage;
    QPoint mousePos;
};

#endif  // MARKAREA_H
