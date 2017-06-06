#ifndef FEATUREBASE_H
#define FEATUREBASE_H

#include <QListWidgetItem>
#include <QStackedLayout>
#include <QLabel>
#include <QString>
#include "devio.h"
#include "device.h"

class FeatureBase : 
    public QObject, 
    public QListWidgetItem
{
    Q_OBJECT
public:
    explicit FeatureBase(Device *dev, QStackedLayout *sl, QLabel* errlabel);
    ~FeatureBase();
    virtual void activate();
    Device *device;
    QLabel* errlabel;
    QStackedLayout* sl;
    QWidget *ui;
    QString description;
    int index;

    void handleConfigChange(shared_ptr<devio::IDevice> dev);

public slots:
    void onUIDestroyed(QObject * obj = NULL);
};

#endif // FEATUREBASE_H
