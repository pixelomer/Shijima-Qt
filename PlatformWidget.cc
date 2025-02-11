#include "PlatformWidget.hpp"
#include "Platform/Platform.hpp"

PlatformWidget::PlatformWidget(): QWidget(), m_flags(0) {
    
}

PlatformWidget::PlatformWidget(QWidget *parent): QWidget(parent), m_flags(0) {

}

PlatformWidget::PlatformWidget(QWidget *parent, PlatformWidget::Flags flags):
    QWidget(parent)
{
    m_flags = flags;
}

void PlatformWidget::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    if (m_flags & ShowOnAllDesktops) {
        Platform::showOnAllDesktops(this);
    }
}
