#pragma once

#include <QTextEdit>
#include "register.h"
#include "devio.h"
#include "registerio.h"

class Register;

using devio::IDevice;

class r04 : public QTextEdit,
	public RegisterIO
{
    Q_OBJECT
public:
    explicit r04(Register* base);
protected slots:
    void timer_update();
private:
    Register* base;
    
    void onRegisterChange(bool write, const vector<Byte>&data) override;
};
