#include "CAccelerometr.h"


CAccelerometr::CAccelerometr(CI2cClient& i2c)
    : mI2c(i2c)
    , mLastAxisData(0,0,0)
    , mAxisOffset(0,0,0)
    , mIsPowerOn(false)
    , mIsFoolResolution(false)
    , mRange(RANGE::UNDEFINE)
{
    cacheFlagsFromSensor();
}



void CAccelerometr::
setPowerMode(const bool isActive)
{
    if (isActive != mIsPowerOn)
    {
        int regValue = readDataFromReg(ACCELEROMETR_REGISTERS::POWER_CTRL);

        (isActive)? regValue|= ACCELEROMETR_BITMASK::PWR_MESSURE : regValue &= ~ACCELEROMETR_BITMASK::PWR_MESSURE;
        bool isSuccess = writeDataToReg(ACCELEROMETR_REGISTERS::POWER_CTRL, regValue);

        if (isSuccess)
        {
            mIsPowerOn = isActive;
        }
    }

    std::cout <<"\n mIsPowerOn = ["<<mIsPowerOn<<"]";
}


void CAccelerometr::
setFullResolution(const bool isFoolResolution)
{
    int regValue = readDataFromReg(ACCELEROMETR_REGISTERS::DATAFORMAT);


    (isFoolResolution)? regValue|= ACCELEROMETR_BITMASK::DATA_FORMAT_FULL_RESULUTION : regValue &= ~ACCELEROMETR_BITMASK::DATA_FORMAT_FULL_RESULUTION;

    bool isSuccess = writeDataToReg(ACCELEROMETR_REGISTERS::DATAFORMAT, regValue);

    if (isSuccess)
    {
        mIsFoolResolution = isFoolResolution;
    }

    std::cout <<"\nmIsFoolResolution ["<<mIsFoolResolution<<"]";
}


void CAccelerometr::
setRange(RANGE range)
{
    std::cout<<std::endl << "CAccelerometr::setRange()";
    int regValue = readDataFromReg(ACCELEROMETR_REGISTERS::DATAFORMAT);
    int value = CAccelerometrHelper::rangeToRegValue(range) | (regValue & ~ACCELEROMETR_BITMASK::DATA_FORMAT_RANGE) ;

    bool isSuccess = writeDataToReg(ACCELEROMETR_REGISTERS::DATAFORMAT, value);
    if (isSuccess)
     {
         mRange = range;

     }

     std::cout <<"\nRange is ["<<(int)mRange<<"]";
}


bool CAccelerometr::
setOffset(const CGeometric3dVector& offset)
{
    const bool success =
           writeDataToReg(ACCELEROMETR_REGISTERS::XOFFSET, offset.getXAxis()) &&
           writeDataToReg(ACCELEROMETR_REGISTERS::YOFFSET, offset.getYAxis()) &&
           writeDataToReg(ACCELEROMETR_REGISTERS::ZOFFSET, offset.getZAxis());

    if (success)
    {
        cacheOffsets();
    }

    return success;
}




double CAccelerometr::
convertMessurementToG(const  int messurement)
{
    return messurement * calculateGCoeficient() * CAccelerometrHelper::G ;
}

double CAccelerometr::
convertMessurementToG(double messurement)
{
    return messurement * calculateGCoeficient() * CAccelerometrHelper::G ;
}

int CAccelerometr::
getMesurementSignMask()
{
    return (CAccelerometrHelper::getRangeDevider(mRange)*8) ;
}




void CAccelerometr::
cacheFlagsFromSensor()
{
   cacheRange();
   cacheResolution();
   cachePowerMode();
   cacheOffsets();
}


void CAccelerometr::
cacheRange()
{
     int regValue =  readDataFromReg(ACCELEROMETR_REGISTERS::DATAFORMAT) ;
     RANGE range = static_cast<RANGE>(regValue & DATA_FORMAT_RANGE);
     mRange = range;
}


void CAccelerometr::
cacheResolution()
{
    bool isFoolResolutionSet;
    int regValue = readDataFromReg(ACCELEROMETR_REGISTERS::DATAFORMAT);

    if (regValue && ACCELEROMETR_BITMASK::DATA_FORMAT_FULL_RESULUTION)
    {
        isFoolResolutionSet = true;
    }

    mIsFoolResolution = isFoolResolutionSet;
}


void CAccelerometr::
cachePowerMode()
{
    bool powerOn = false;
    int regValue =  readDataFromReg(ACCELEROMETR_REGISTERS::POWER_CTRL) ;

    if (regValue && ACCELEROMETR_BITMASK::PWR_MESSURE)
    {
        powerOn = true;
    }

    mIsPowerOn = powerOn;
}


void CAccelerometr::
cacheOffsets()
{

    mAxisOffset.setXAxis(readDataFromReg(ACCELEROMETR_REGISTERS::XOFFSET)) ;

    mAxisOffset.setYAxis(readDataFromReg(ACCELEROMETR_REGISTERS::YOFFSET)) ; ;

    mAxisOffset.setZAxis(readDataFromReg(ACCELEROMETR_REGISTERS::ZOFFSET)) ; ;
}


bool CAccelerometr::
writeDataToReg(const ACCELEROMETR_REGISTERS  reg, const int value)
{
    std::cout <<std::endl << "Write to "<< (int) reg<< " value " << value;
    bool isNoError = false;

    if(CPositionHelper::isCorrect8bit(value))
    {
        const std::string valueToWrite = mI2c.convertToString(static_cast<uint8> (value));
        isNoError = mI2c.writeDataToI2cRegister(accelerometrAddress, static_cast<uint8> (reg),valueToWrite );
    }

    return  isNoError;
}


int CAccelerometr::
readDataFromReg(const ACCELEROMETR_REGISTERS reg)
{
    const std::string regValue = mI2c.readDataFromI2cRegister(accelerometrAddress, static_cast<uint8> (reg), 1);
    std::cout << std::endl << "Read from "<< (int) reg<<" value " <<(int) regValue[0];
    return mI2c.convertToInt(regValue);
}



 void CAccelerometr::
 readDownAllAxis()
 {
    const int nBytes=6;
    const std::string axisData = mI2c.readDataFromI2cRegister(accelerometrAddress, static_cast<uint8> (ACCELEROMETR_REGISTERS::XAcisL), nBytes);


    if (axisData.size()==0)
    {
        std::cout<<"Data wasn't recieved correct\n";
    }
    else
    {
        std::vector<short int> axisInfoVector;
        for (int i=0; i<nBytes; i=i+2)
        {
           short int symb = axisData[i+1]<<8 | axisData[i];
            const double dataAxis = convertMessurementToG (symb) ;
            axisInfoVector.push_back(symb);
            std::cout<<dataAxis<<" ";
        }

        CGeometric3dVector vector(axisInfoVector);

        mLastAxisData = vector;

 //       std::cout<<"\n Accel ="<< convertMessurementToG(mLastAxisData.lenght()) ;
        std::cout<<"  xAng ="<< CGeometric3dVector::cosToDegree(mLastAxisData.angleX())
                 <<"  yAng ="<< CGeometric3dVector::cosToDegree(mLastAxisData.angleY())
                 <<"  zAng ="<< CGeometric3dVector::cosToDegree(mLastAxisData.angleZ())
                 << std::endl;

    }
 }


 double CAccelerometr::
 calculateGCoeficient() const
 {

     float coef = 0 ;
     if (mIsFoolResolution)
     {
        coef = CAccelerometrHelper::calibratedCoefOfG ;
     }
     else
     {
        coef = CAccelerometrHelper::calibratedCoefOfG * ( CAccelerometrHelper::getRangeDevider(mRange) / 2 )  ;
     }


     return coef;
 }
