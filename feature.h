#ifndef FEATURE_H
#define FEATURE_H

#include <QListWidgetItem>
#include <QStackedLayout>
#include <QLabel>
#include <QString>
#include "devio.h"
#include "device.h"
#include "featurebase.h"

class Feature :
    public FeatureBase, 
    public devio::Subscriber<devio::IFeature0020ConfigChange::ConfigReport>
{
    Q_OBJECT
public:
    explicit Feature(Device *dev, QStackedLayout *sl, shared_ptr<devio::IFeature> f, QLabel* errlabel);
    void activate() override;
    void settings_lost();
    void onCookieSet(unsigned int swid, unsigned short cookie) override;
private:
    shared_ptr<devio::IFeature> feature;

public:
    static string feature_name(int number);
    static bool isBazingaDFUFeature(const int feature_id);
    
signals:
    
};

#endif // FEATURE_H

/*
#include <iostream>
#include <thread>

using namespace std;

void test_func()
{
  // do something
  double dSum = 0;
  for( int i = 0; i < 10000; ++ i )
    for( int j = 0; j < 10000; ++ j )
      dSum += i*j;

  cout << "Thread: " << dSum << endl;
}

int main( int argc, char** argv )
{
  // execute thread
  thread mThread( test_func );

  // do somthing
  cout << "main thread" << endl;

  // wait the thread stop
  mThread.join();

  return 0;
}*/

    /* testing strings for thread
    for (int i = 1; i <= 10; i++){
        for (int j = 1; j <= 10; j++){
            std::string s = std::to_string(i*j);
            std::wstring stemp = std::wstring(s.begin(), s.end());
            LPCWSTR sw = stemp.c_str();
            OutputDebugStringW(sw);
            OutputDebugStringW(L"\n");
        }
    }*/