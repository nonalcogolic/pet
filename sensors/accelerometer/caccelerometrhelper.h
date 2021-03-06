#ifndef CACCELEROMETRHELPER_H
#define CACCELEROMETRHELPER_H

enum class ACCELEROMETR_REGISTERS
{
    WHO_I_AM = 0,
    XOFFSET = 0x1E,
    YOFFSET = 0x1F,
    ZOFFSET = 0x20,
    BW_RATE = 0x2C,
    POWER_CTRL = 0x2D,
    DATAFORMAT = 0x31,
    XAcisL = 0x32,
    XAcisH = 0x33,
    YAcisL = 0x34,
    YAcisH = 0x35,
    ZAcisL = 0x36,
    ZAcisH = 0x37

};


enum class RANGE
{
    UNDEFINE,
    _2G = 2 ,
    _4G = 4 ,
    _8G = 8 ,
    _16G = 16
};


enum class RATE
{
    _1600,
    _800,
    _400,
    _200,
    _100
};



enum ACCELEROMETR_BITMASK
{
    PWR_MESSURE = 0x08,
    DATA_FORMAT_FULL_RESULUTION = 0x08,
    DATA_FORMAT_RANGE = 0x03,
    BW_RATE_RATE = 0x0F
};







namespace CAccelerometrHelper
{
   const  double G = 9.8;
   const double calibratedCoefOfG = 0.0040239;

   int getRangeDevider(const RANGE& range);
   int rangeToRegValue(const RANGE& range);
   RANGE convertRegValueToRange(const int & regValue);

   int rateToRegValue(const RATE & rate);
   RATE convertRegValueToRate(const int & regValue);
   int convertRateToDelay(const RATE& rate);
}

#endif // CACCELEROMETRHELPER_H
