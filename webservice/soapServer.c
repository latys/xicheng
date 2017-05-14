/* soapServer.c
   Generated by gSOAP 2.8.37 for SynchronizedAnaData.h

gSOAP XML Web services tools
Copyright (C) 2000-2016, Robert van Engelen, Genivia Inc. All Rights Reserved.
The soapcpp2 tool and its generated software are released under the GPL.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
--------------------------------------------------------------------------------
A commercial use license is available from Genivia Inc., contact@genivia.com
--------------------------------------------------------------------------------
*/

#if defined(__BORLANDC__)
#pragma option push -w-8060
#pragma option push -w-8004
#endif
#include "soapH.h"
#ifdef __cplusplus
extern "C" {
#endif

SOAP_SOURCE_STAMP("@(#) soapServer.c ver 2.8.37 2016-11-05 07:27:00 GMT")
SOAP_FMAC5 int SOAP_FMAC6 soap_serve(struct soap *soap)
{
#ifndef WITH_FASTCGI
	soap->keep_alive = soap->max_keep_alive + 1;
#endif
	do
	{
#ifndef WITH_FASTCGI
		if (soap->keep_alive > 0 && soap->max_keep_alive > 0)
			soap->keep_alive--;
#endif
		if (soap_begin_serve(soap))
		{	if (soap->error >= SOAP_STOP)
				continue;
			return soap->error;
		}
		if ((soap_serve_request(soap) || (soap->fserveloop && soap->fserveloop(soap))) && soap->error && soap->error < SOAP_STOP)
		{
#ifdef WITH_FASTCGI
			soap_send_fault(soap);
#else
			return soap_send_fault(soap);
#endif
		}
#ifdef WITH_FASTCGI
		soap_destroy(soap);
		soap_end(soap);
	} while (1);
#else
	} while (soap->keep_alive);
#endif
	return SOAP_OK;
}

#ifndef WITH_NOSERVEREQUEST
SOAP_FMAC5 int SOAP_FMAC6 soap_serve_request(struct soap *soap)
{
	soap_peek_element(soap);
	if (!soap_match_tag(soap, soap->tag, "ns1:synchronizedAnaData"))
		return soap_serve_ns1__synchronizedAnaData(soap);
	return soap->error = SOAP_NO_METHOD;
}
#endif

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ns1__synchronizedAnaData(struct soap *soap)
{	struct ns1__synchronizedAnaData soap_tmp_ns1__synchronizedAnaData;
	struct ns1__synchronizedAnaDataResponse soap_tmp_ns1__synchronizedAnaDataResponse;
	enum xsd__boolean soap_tmp_xsd__boolean;
	soap_default_ns1__synchronizedAnaDataResponse(soap, &soap_tmp_ns1__synchronizedAnaDataResponse);
	soap_default_xsd__boolean(soap, &soap_tmp_xsd__boolean);
	soap_tmp_ns1__synchronizedAnaDataResponse._synchronizedAnaDataReturn = &soap_tmp_xsd__boolean;
	soap_default_ns1__synchronizedAnaData(soap, &soap_tmp_ns1__synchronizedAnaData);
	if (!soap_get_ns1__synchronizedAnaData(soap, &soap_tmp_ns1__synchronizedAnaData, "ns1:synchronizedAnaData", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = ns1__synchronizedAnaData(soap, soap_tmp_ns1__synchronizedAnaData._gateId, soap_tmp_ns1__synchronizedAnaData._equipId, soap_tmp_ns1__synchronizedAnaData._data, soap_tmp_ns1__synchronizedAnaData._equipIp, soap_tmp_ns1__synchronizedAnaData._equipDk, soap_tmp_ns1__synchronizedAnaDataResponse._synchronizedAnaDataReturn);
	if (soap->error)
		return soap->error;
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__synchronizedAnaDataResponse(soap, &soap_tmp_ns1__synchronizedAnaDataResponse);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__synchronizedAnaDataResponse(soap, &soap_tmp_ns1__synchronizedAnaDataResponse, "ns1:synchronizedAnaDataResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns1__synchronizedAnaDataResponse(soap, &soap_tmp_ns1__synchronizedAnaDataResponse, "ns1:synchronizedAnaDataResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

#ifdef __cplusplus
}
#endif

#if defined(__BORLANDC__)
#pragma option pop
#pragma option pop
#endif

/* End of soapServer.c */
