#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <QString>
#include <QtGlobal>

#include "i2ceeprom_p.h"
#include "F24LC256_p.h"


cF24LC256Private::cF24LC256Private(QString devNode, int dLevel, short adr)
    : cI2CEEPromPrivate(devNode,dLevel,adr)
{
}


int cF24LC256Private::WriteData(char* data, ushort count, ushort adr)
{
    uchar outpBuf[66]; // 2 adr byte, max 64 byte data
    struct i2c_msg Msgs = {addr: I2CAdress, flags: I2C_M_RD, len: 5, buf:  outpBuf }; // 1 message
    struct i2c_rdwr_ioctl_data EEPromData = { msgs: &(Msgs), nmsgs: 1 };
    int toWrite = count;

    if ( I2CTransfer(DevNode,I2CAdress,DebugLevel,&EEPromData) )
    return 0; // we couldn't write any data

    Msgs.flags = 0; // switch to write direction
    char* mydata = data;
    while (toWrite)
    {
        if (adr > 32767 ) break; // we are ready if adress get greater than 32767
        outpBuf[0] = (adr >> 8) & 0xff; outpBuf[1] = adr & 0xff;
        int pl = 64 - (adr & 0x3f); // how many bytes for the actual page
        int l = (toWrite > pl) ? pl : toWrite; // so we decide how many bytes to write now
        memcpy((void*)&outpBuf[2],(void*)mydata,l);
        mydata+=l;
        Msgs.len = l+2; // set length for i2c driver
        int r;
        while (( r = I2CTransfer(DevNode,I2CAdress,DebugLevel,&EEPromData)) == 2); // device node ok , but eeprom is busy
        if (r) break; // problems when writing to device node
        adr += l; // set adress where to go on
        toWrite -= l; // actualize byte to write
    }
    return(count - toWrite);
}


int cF24LC256Private::ReadData(char* data, ushort count, ushort adr)
{
    uchar outpBuf[2];
    uchar inpBuf[count];
    ushort ret = 0;
    struct i2c_msg Msgs[2] = { {addr :I2CAdress, flags: 0,len: 2,buf: &(outpBuf[0])}, // 2 messages (tagged format )
              {addr :I2CAdress, flags: (I2C_M_RD+I2C_M_NOSTART), len: count, buf: &(inpBuf[0])} };

    struct i2c_rdwr_ioctl_data EEPromData = {  msgs: &(Msgs[0]), nmsgs: 2 };

    outpBuf[0]=(adr >> 8) & 0xff; outpBuf[1]=adr & 0xff;

    if ( I2CTransfer(DevNode,I2CAdress,DebugLevel,&EEPromData) == 0 )
    {   // everything ok and read
        memcpy((void*)data,(void*)&inpBuf[0],count);
        ret = count;
    }

    return ret;
}

int cF24LC256Private::size() {
    return 32768;
}

