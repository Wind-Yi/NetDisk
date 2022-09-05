#include "pdu.h"
#include "stdlib.h"
#include "string.h"

PDU *mkPDU(uint uiMsgLen)
{
    uint uiPDULen = uiMsgLen + sizeof(PDU);
    PDU* pdu = (PDU*)malloc(uiPDULen);
    if(NULL==pdu)
    {
        exit(EXIT_FAILURE);
    }
    memset(pdu, 0, uiPDULen);
    pdu->uiMsgLen = uiMsgLen;
    pdu->uiPDULen = uiPDULen;
    return pdu;
}
