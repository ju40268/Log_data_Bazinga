#ifndef F6500_H
#define F6500_H

#include <QTextEdit>
#include "feature.h"
#include "devio.h"

using devio::IFeature6500Gestures;
using devio::Subscriber;
using devio::IFeature;

class f6500 : public QTextEdit,
        public Subscriber<devio::IFeature6500Gestures::Report>
{
    Q_OBJECT
public:
    explicit f6500(shared_ptr<devio::IFeature6500Gestures> f, Feature* base);
    ~f6500();
	void OnGestureScrollEvent(GestureId gesture_id, EventType event_type, int dx, int dy, int period ) override;
    
signals:
    void signalGestureScrollEvent(GestureId gesture_id, EventType event_type, int dx, int dy, int period );

private slots:
    void slotGestureScrollEvent(GestureId gesture_id, EventType event_type, int dx, int dy, int period );

private:
    shared_ptr<devio::IFeature6500Gestures> feature6500;
    Feature* baseFeature;
};

#endif // F6500_H
