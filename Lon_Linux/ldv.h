/*
// $Header: /home/garyb/FlashStick/CVSROOT/DigitalHome/UsbLta/ldv.h,v 1.3 2005/06/26 23:02:43 garyb Exp $
//
// (c) Copyright 2005, Gary Bartlett.  All rights reserved.
//
// USB LonTalk Adapter - LonWorks Driver API
*/

/*!
 * @header
 * @abstract    USB LonTalk Adapter Driver API
 * @discussion  The LDV Layer is (currently) a single-threaded,
 *              semi-synchronous layer, that is responsible for
 *              converting between LDV and LLP packets, and responsing
 *              to certain local network interface messages (not yet
 *              implemented), such as Identify and/or Wink.
 *              Calls to read LDV data are non-blocking when no data is
 *              available but will block once a message starts to arrive and
 *              will only return when the complete message is received or an
 *              error/timeout occurs. Callers can either poll for available
 *              data or wait on the LDV handle by calling select().
 */

#ifndef _LDV_H_
#define _LDV_H_


/*!
 * @typedef     LDV_HANDLE
 * @abstract    LDV device handle
 * @discussion  Returned by ldv_open and used to identify device in all other
 *              LDV functions.
 */
typedef short   LDV_HANDLE;

/*!
 * @typedef     LDV_STATUS
 * @abstract    LDV return/status code.
 * @discussion  Returned by all (most) LDV functions, indicating success or
 *              failure of the performed operation.
 * @constant    LDV_OK                      success
 * @constant    LDV_INVALID_DEVICE_ID       an invalid device name or handle was specified
 * @constant    LDV_INVALID_PARAMETER       an invalid parameter was specified
 * @constant    LDV_OPEN_FAILED             a failure occurred whilst opening the device
 * @constant    LDV_CLOSE_FAILED            a failure occurred whilst closing the device
 * @constant    LDV_NO_MSG_AVAIL            there are no messages available at this time
 * @constant    LDV_READ_FAILED             a failure occurred whilst reading from the device
 * @constant    LDV_WRITE_FAILED            a failure occurred whilst writing to the device
 * @constant    LDV_INVALID_BUF_LEN         the specified read buffer was too small
 * @constant    LDV_INVALID_MESSAGE_LENGTH  the length of the specified message was invalid
 * @constant    LDV_WAIT_FAILED             a failure occurred whilst waiting for data from the device
 * @constant    LDV_INVALID_MESSAGE         an invalid message was received from the device
 * @constant    LDV_INTERNAL_ERROR          an internal error occurred
 * @constant    LDV_COMPLETION_FAILED       a failed completion occurred whilst writing to the device
 * @constant    LDV_TIMEOUT                 a timeout occurred whilst waiting for completion
 */
enum LdvCode
{
    LDV_OK                          =  0,
    LDV_NOT_FOUND                   =  1,
    LDV_ALREADY_OPEN                =  2,
    LDV_NOT_OPEN                    =  3,
    LDV_DEVICE_ERR                  =  4,
    LDV_INVALID_DEVICE_ID           =  5,
    LDV_NO_MSG_AVAIL                =  6,
    LDV_NO_BUFF_AVAIL               =  7,
    LDV_NO_RESOURCES                =  8,
    LDV_INVALID_BUF_LEN             =  9,
    LDV_NOT_ENABLED                 = 10,
    
    /* added in OpenLDV 1.0 */
    LDV_INITIALIZATION_FAILED       = 11,
    LDV_OPEN_FAILED                 = 12,
    LDV_CLOSE_FAILED                = 13,
    LDV_READ_FAILED                 = 14,
    LDV_WRITE_FAILED                = 15,
    LDV_REGISTER_FAILED             = 16,
    LDV_INVALID_XDRIVER             = 17,
    LDV_DEBUG_FAILED                = 18,
    LDV_ACCESS_DENIED               = 19,
    
    /* added in OpenLDV 2.0 */
    LDV_CAPABLE_DEVICE_NOT_FOUND    = 20,
    LDV_NO_MORE_CAPABLE_DEVICES     = 21,
    LDV_CAPABILITY_NOT_SUPPORTED    = 22,
    LDV_INVALID_DRIVER_INFO         = 23,
    LDV_INVALID_DEVICE_INFO         = 24,
    LDV_DEVICE_IN_USE               = 25,
    LDV_NOT_IMPLEMENTED             = 26,
    LDV_INVALID_PARAMETER           = 27,
    LDV_INVALID_DRIVER_ID           = 28,
    LDV_INVALID_DATA_FORMAT         = 29,
    LDV_INTERNAL_ERROR              = 30,
    LDV_EXCEPTION                   = 31,
    LDV_DRIVER_UPDATE_FAILED        = 32,
    LDV_DEVICE_UPDATE_FAILED        = 33,
    LDV_STD_DRIVER_TYPE_READ_ONLY   = 34,
    
    /* added post OpenLDV 2.1 */
    LDV_INVALID_MESSAGE             = 35,
    LDV_INVALID_MESSAGE_LENGTH      = 36,
    LDV_WAIT_FAILED                 = 37,
    LDV_COMPLETION_FAILED           = 38,
    LDV_TIMEOUT                     = 39
};

typedef short                       LDV_STATUS;


#ifdef __cplusplus
extern "C" {
#endif
    
    /*!
     * @function 
     * @abstract        Open the USBLTA with the specified serial number.
     * @param szSerial  Serial number of the USBLTA to be opened.
     * @param phLdv     Pointer to variable to receive handle of opened USBLTA adapter.
     * @result          LDV_OK if successful, or other LDV_STATUS value.
     */
    LDV_STATUS ldv_open(const char* szSerial, LDV_HANDLE* phLdv);

    /*!
     * @function 
     * @abstract        Close a USBLTA.
     * @param hLdv      Handle of the USBLTA to be closed.
     * @result          LDV_OK if successful, or other LDV_STATUS value.
     */
    LDV_STATUS ldv_close(LDV_HANDLE hLdv);

    /*!
     * @function 
     * @abstract        Read a single LDV message from a USBLTA.
     * @param hLdv      Handle of the USBLTA to read a message from.
     * @param pBuf      Pointer to a buffer to receive the next LDV message.
     * @param nBuf      Length of the buffer pointed to by pBuf.
     * @result          LDV_OK if successful, or other LDV_STATUS value.
     */
    LDV_STATUS ldv_read(LDV_HANDLE hLdv, void* pLdvBuf, size_t nLdvBuf);

    /*!
     * @function 
     * @abstract        Write LDV message(s) to a USBLTA.
     * @param hLdv      Handle of the USBLTA to write message(s) to.
     * @param pMsg      Pointer to a buffer containing the LDV message(s)
     *                  to be written to the USBLTA.
     * @param nMsg      Total length of the LDV message(s) pointed to by pMsg.
     * @result          LDV_OK if successful, or other LDV_STATUS value.
     */
    LDV_STATUS ldv_write(LDV_HANDLE hLdv, const void* pLdvMsg, size_t nLdvMsg);


    /*!
     * @function 
     * @abstract        Return the total length of an LDV message.
     * @param pMsg      Pointer to a buffer containing at least one LDV message.
     * @result          Length of the first LDV message found.
     */
    size_t ldv_len(const void* pLdvMsg);

    
    /*!
     * @function 
     * @abstract        Wait for data to arrive from a USBLTA.  This call will
     *                  block until the first byte has arrived.  At this point
     *                  a call to ldv_read will block until the entire
     *                  message has arrived or a failure occurs, so it's
     *                  possible that a subsequent ldv_read will return
     *                  LDV_NO_MSG_AVAIL.
     * @param hLdv      Handle of the USBLTA to wait on.
     * @param nWaitMs   The maximum length of time to wait, in milliseconds.
     *                  Specify LDV_WAIT_FOREVER to not timeout.
     * @result          LDV_OK if data has arrived, LDV_NO_MSG_AVAIL if
     *                  no data has arrived before the specified timeout,
     *                  or other LDV_STATUS value on failure.
     */
    LDV_STATUS ldv_wait(LDV_HANDLE hLdv, unsigned nWaitMs);
    
    static const unsigned LDV_WAIT_FOREVER = (unsigned)-1;
    
#ifdef __cplusplus
}
#endif

#endif /*_LDV_H_*/
