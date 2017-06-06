#include "feature.h"
#include "f4540.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"

using std::string;
using std::vector;
using devio::Byte;



//*************************************************************************
//
// f4540::f4540
//
//*************************************************************************

f4540::f4540(shared_ptr<devio::IFeature4540KBLayout> f, Feature* base) :
    QTextEdit(),
    feature4540(f), 
    baseFeature(base)
{
    setPlainText(baseFeature->description);
    setReadOnly(true);
    setWordWrapMode(QTextOption::NoWrap);

    vector<string> output;
    int layout=0;
    bool ok1 = feature4540->GetKeyboardLayout( layout);

    append( stringbuilder()
        << "GetKeyboardLayout("
        << " layout=" << layout
        << ") -> " << describe_err(ok1) << "\n"
        );

    append( stringbuilder() 
        << "\t1 - US\n"
        << "\t2 - US International (105 Keys)\n"
        << "\t3 - UK\n"
        << "\t4 - German\n"
        << "\t5 - French\n"
        << "\t6 - German/French\n"
        << "\t7 - Russian\n"
        << "\t8 - Pan Nordic\n"
        << "\t9 - Korean\n"
        << "\t10 - Japanese\n"
        << "\t11 - Chinese Traditional\n"
        << "\t12 - Chinese Simplified\n"
        << "\t13 - Swiss\n"
        << "\t14 - Turkish\n"
        << "\t15 - Spanish\n"
        << "\t16 - Arabic\n"
        << "\t17 - Belgian\n"
        << "\t18 - Canadian Bilingual\n"
        << "\t19 - Canadian French\n"
        << "\t20 - Czech\n"
        << "\t21 - Danish\n"
        << "\t22 - Finnish\n"
        << "\t23 - Greek\n"
        << "\t24 - Hebrew\n"
        << "\t25 - Hungary\n"
        << "\t26 - Italian\n"
        << "\t27 - Latin American\n"
        << "\t28 - Netherlands - Dutch\n"
        << "\t29 - Norwegian\n"
        << "\t30 - Poland\n"
        << "\t31 - Portuguese\n"
        << "\t32 - Slovakia\n"
        << "\t33 - Swedish\n"
        << "\t34 - Swiss French\n"
        << "\t35 - Yugoslavia\n"
        << "\t36 - Turkish-F\n"
        << "\t37 - Icelandic\n"
        << "\t38 - Romanian\n"
        << "\t39 - Latvian\n"
        << "\t40 - Bulgarian\n"
        << "\t41 - Estonian\n"
        << "\t42 - Lithuanian\n"
        << "\t43 - Serbian Cyrillic\n"
        << "\t44 - Slavonic Multilingual\n"
        << "\t45 - Ukrainian\n"
        << "\t46 - Kazakh\n"
        << "\t47 - Chinese Cangjie\n"
        << "\t48 - Chinese Zhuyin\n"
        << "\t49 - Chinese Dayi\n"
        << "\t50 - Chinese Wubi\n"
        << "\t51 - Thai\n"
        << "\t52 - Vietnamese\n"
        << "\t53 - Malaysia\n"
        << "\t54 - India\n"
        << "\t55 - US International (104 Keys)\n");
}
