//*************************************************************************
//
// f8070SyncModel.h
//
//*************************************************************************

#pragma once

#include <QStandardItemModel>

class f8070SyncModel : public QStandardItemModel
{
    Q_OBJECT

    enum Column
    {
        ColumnSyncEventZoneIndex,
        ColumnSyncEventPeriod,
        ColumnSyncHostDriftInMs,
        ColumnSyncHostDriftPercentage,
        ColumnSyncMsSinceLastReport,
        ColumnSyncSentSyncRequest,
        ColumnCount,
    };

public:
    f8070SyncModel(QObject *parent = NULL)
        : QStandardItemModel(0, ColumnCount, parent)
    {
        setHorizontalHeaderItem(ColumnSyncEventZoneIndex, new QStandardItem("Zone Index"));
        setHorizontalHeaderItem(ColumnSyncEventPeriod, new QStandardItem("Effect Position (ms)"));
        setHorizontalHeaderItem(ColumnSyncHostDriftInMs, new QStandardItem("Host Drift (ms)"));
        setHorizontalHeaderItem(ColumnSyncHostDriftPercentage, new QStandardItem("Host Drift (%)"));
        setHorizontalHeaderItem(ColumnSyncMsSinceLastReport, new QStandardItem("Event Interval (ms)"));
        setHorizontalHeaderItem(ColumnSyncSentSyncRequest, new QStandardItem("Sync Requested?"));
    }

    void addSyncEvent(quint32 zoneIndex, quint32 eventPosInMs, qint32 hostDriftInMs, float hostDriftPercentage, quint32 msSinceLastReport, bool sentSyncRequest)
    {
        quint32 rowIndex = rowCount();

        QList<QStandardItem*> columnData;

        for (int i = ColumnSyncEventZoneIndex; i < ColumnCount; i++)
        {
            QStandardItem *item = new QStandardItem;
            switch (i)
            {
                case ColumnSyncEventZoneIndex:
                    item->setText(QString("%1").arg(zoneIndex));
                    break;
                case ColumnSyncEventPeriod:
                    item->setText(QString("%1").arg(eventPosInMs));
                    break;
                case ColumnSyncHostDriftInMs:
                    item->setText(QString("%1").arg(hostDriftInMs));
                    break;
                case ColumnSyncHostDriftPercentage:
                    item->setText(QString("%1%").arg(hostDriftPercentage, 0, 'f', 2));
                    break;
                case ColumnSyncMsSinceLastReport:
                    item->setText(QString("%1").arg(msSinceLastReport));
                    break;
                case ColumnSyncSentSyncRequest:
                    item->setText(QString("%1").arg(sentSyncRequest));
                    break;
                default:
                    break;
            }
            columnData << item;
        }

        insertRow(rowIndex, columnData);
    }
};
