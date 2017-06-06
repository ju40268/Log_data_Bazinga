#pragma once

#include <QTextEdit>
#include "register.h"
#include "devio.h"
#include "registerio.h"

class Register;

using devio::IDevice;

class r02 : public QTextEdit,
	public RegisterIO
{
    Q_OBJECT
public:
    explicit r02(Register* base);
private:
    Register* base;
    
    void onRegisterChange(bool write, const vector<Byte>&data) override;
};
