#pragma once

#include <QTextEdit>
#include "registerio.h"
#include "devio.h"

using devio::IDevice;
class Register;

class r01 : public QTextEdit,
    public RegisterIO
{
    Q_OBJECT
public:
    explicit r01(Register* base);
private:
    Register* base;
    
    void onRegisterChange(bool write, const vector<Byte>&data) override;
};
