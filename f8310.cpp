#include "feature.h"
#include "f8310.h"
#include <QMenu>
#include <QGroupBox>
#include <QValidator>
#include "xlog/xlog.h"

f8310::f8310(shared_ptr<devio::IFeature8310Equalizer> f, Feature* base) :
QWidget(),
feature8310(f),
baseFeature(base)
{
    createUI();
}


void f8310::createUI(void)
{
    QVBoxLayout* vLayout = new QVBoxLayout;
    setLayout(vLayout);

    {
        QGroupBox* group = new QGroupBox("EQ");
        QVBoxLayout* dataLayout = new QVBoxLayout;
        group->setLayout(dataLayout);

        int bandCount = 0;
        int dbRange = 0;
        int capabilities = 0;
        if (!feature8310->getEQInfo(bandCount, dbRange, capabilities))
        {
            return;
        }

        QLabel *infoLabel = new QLabel;
        infoLabel->setText(QString("%1 band frequencies with -%2db to %2db range").arg(bandCount).arg(dbRange));
        dataLayout->addWidget(infoLabel);

        bool hasGainCapability = (capabilities & IFeature8310Equalizer::EqCapability::SET_THROUGH_FREQUENCY_GAINS);
        bool hasCoefficientsCapability = (capabilities & IFeature8310Equalizer::EqCapability::SET_THROUGH_FREQUENCY_COEFFICIENTS);

        int startIdx = 0;
        int readCount = 0;
        vector<int> frequencies;
        QList<int> allFrequencies;
        while (readCount < bandCount)
        {
            if (feature8310->getFrequencies(startIdx, frequencies))
            {
                readCount += (int)frequencies.size();
                startIdx += (int)frequencies.size();
                allFrequencies.append(QList<int>::fromVector(QVector<int>::fromStdVector(frequencies)));
            }
            else
            {
                return;
            }
        }
        if (allFrequencies.size() != bandCount)
        {
            Q_ASSERT(false);
            return;
        }

        QHBoxLayout* hLayout = new QHBoxLayout;

        for (int i = 0; i < bandCount; i++)
        {
            int frequencyValue = allFrequencies.at(i);
            QVBoxLayout *freqVLayout = new QVBoxLayout;
            QLabel *frequency = new QLabel(QString("%1").arg(frequencyValue));
            QSlider *vSlider = new QSlider(Qt::Vertical);
            vSlider->setTracking(false);
            int m = -dbRange;
            vSlider->setRange(m, dbRange);
            connect(vSlider, SIGNAL(valueChanged(int)), this, SLOT(onGainChanged(int)));

            gainSliderList.append(vSlider);

            freqVLayout->addWidget(frequency);
            freqVLayout->addWidget(vSlider);

            hLayout->addLayout(freqVLayout);
        }
        dataLayout->addLayout(hLayout);

        vLayout->addWidget(group);
    }

    // Get current settings
    readFrequencyGains();
}

//*************************************************************************
//
// f8310::readFrequencyGains
//
//*************************************************************************

void f8310::readFrequencyGains(void)
{
    if (feature8310)
    {
        vector<int> gains;
        if (feature8310->getFrequencyGains(gains))
        {
            QList<int> gainList = QList<int>::fromVector(QVector<int>::fromStdVector(gains));
            if (gainList.size() >= gainSliderList.size())
            {
                for (int i = 0; i < gainSliderList.size(); i++)
                {
                    bool b = gainSliderList[i]->blockSignals(true);
                    gainSliderList[i]->setValue(gainList[i]);
                    gainSliderList[i]->blockSignals(b);
                }
            }
        }
    }
}

//*************************************************************************
//
// f8310::onGainChanged
//
//*************************************************************************

void f8310::onGainChanged(int /*gain*/)
{
    if (!feature8310)
    {
        return;
    }

    vector<int> gainList;
    for (int i = 0; i < gainSliderList.size(); i++)
    {
        gainList.push_back(gainSliderList[i]->value());
    }

    feature8310->setFrequencyGains(2, gainList);
}

