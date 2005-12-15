/****************************************************************************
** $file: amanith/src/gerror.cpp   0.2.0.0   edited Dec, 12 2005
**
** Global error codes.
**
**
** Copyright (C) 2004-2005 Mazatech Inc. All rights reserved.
**
** This file is part of Amanith Framework.
**
** This file may be distributed and/or modified under the terms of the Q Public License
** as defined by Mazatech Inc. of Italy and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Amanith Professional Edition license may use this file in
** accordance with the Amanith Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.mazatech.com or email sales@mazatech.com for
** information about Amanith Commercial License Agreements.
** See http://www.amanith.org/ for opensource version, public forums and news.
**
** Contact info@mazatech.com if any conditions of this licensing are
** not clear to you.
**********************************************************************/

#include "amanith/gerror.h"

#if G_USED_LANG == G_LANG_IT
	#include "amanith/lang/gerror_it.h"
#elif G_USED_LANG == G_LANG_EN
	#include "amanith/lang/gerror_en.h"
#else
	#include "amanith/lang/gerror_en.h"
#endif

namespace Amanith {

GString ErrorUtils::ErrToString(const GError ErrorCode) {

	switch (ErrorCode) {
		case G_NO_ERROR:
			return G_NO_ERROR_STR;
		//! Unknown and unespected error
		case G_UNKNOWN_ERROR:
			return G_UNKNOWN_ERROR_STR;
		//! File was not found
		case G_FILE_NOT_FOUND:
			return G_FILE_NOT_FOUND_STR;
		//! File already exist
		case G_FILE_ALREADY_EXISTS:
			return G_FILE_ALREADY_EXISTS_STR;
		//! Invalid file format (ex: unsupported image file)
		case G_INVALID_FORMAT:
			return G_INVALID_FORMAT_STR;
		//! Permission denied
		case G_PERMISSION_DENIED:
			return G_PERMISSION_DENIED_STR;
		//! Invalid parameter (maybe a NULL pointer, or wrong number)
		case G_INVALID_PARAMETER:
			return G_INVALID_PARAMETER_STR;
		//! Invalid XML node
		case G_INVALID_NODE:
			return G_INVALID_NODE_STR;
		//! Passed ClassID or GElement is not supported for the specified operation
		case G_UNSUPPORTED_CLASSID:
			return G_UNSUPPORTED_CLASSID_STR;
		//! Specified index is out of range
		case G_OUT_OF_RANGE:
			return G_OUT_OF_RANGE_STR;
		//! Generic stream read error
		case G_READ_ERROR:
			return G_READ_ERROR_STR;
		//! Generic stream write error
		case G_WRITE_ERROR:
			return G_WRITE_ERROR_STR;
		//! Generic memory (allocation/deallocation) error
		case G_MEMORY_ERROR:
			return G_MEMORY_ERROR_STR;
		//! Class cannot do operation because a missing kernel owner
		case G_MISSING_KERNEL:
			return G_MISSING_KERNEL_STR;
		//! Plugin missed feature error
		case G_MISSED_FEATURE:
			return G_MISSED_FEATURE_STR;
		//! Entry already exists
		case G_ENTRY_ALREADY_EXISTS:
			return G_ENTRY_ALREADY_EXISTS_STR;
		//! Invalid operation
		case G_INVALID_OPERATION:
			return G_INVALID_OPERATION_STR;
		//! Plugin is not loaded
		case G_PLUGIN_NOTLOADED:
			return G_PLUGIN_NOTLOADED_STR;
		//! Plugin not present
		case G_PLUGIN_NOTPRESENT:
			return G_PLUGIN_NOTPRESENT_STR;
		//! Plug symbol not resolved
		case G_PLUGIN_SYMBOL_UNRESOLVED:
			return G_PLUGIN_SYMBOL_UNRESOLVED_STR;
		//! Plugin is already loaded
		case G_PLUGIN_ALREADYLOADED:
			return G_PLUGIN_ALREADYLOADED_STR;
		//! Plugin has created some instances, so it can't be unload
		case G_PLUGIN_ISTANCED:
			return G_PLUGIN_ISTANCED_STR;

		// *********************************************************************
		//                           SMTP error codes
		// *********************************************************************
		case G_SMTP_UNEXPECTEDANSWER:
			return G_SMTP_UNEXPECTEDANSWER_STR;
		case G_SMTP_SERVICECLOSED:
			return G_SMTP_SERVICECLOSED_STR;
		case G_SMTP_NOTIMPLEMENTED:
			return G_SMTP_NOTIMPLEMENTED_STR;
		case G_SMTP_MIMENOTSUPPORTED:
			return G_SMTP_MIMENOTSUPPORTED_STR;
		case G_SMTP_SERVERCANTEXECUTE:
			return G_SMTP_SERVERCANTEXECUTE_STR;
		case G_SMTP_CANTCONNECT:
			return G_SMTP_CANTCONNECT_STR;
		case G_SMTP_DATAERROR:
			return G_SMTP_DATAERROR_STR;
		case G_SMTP_SYNTAXERROR:
			return G_SMTP_SYNTAXERROR_STR;
		case G_SMTP_STORAGEEXCEDED:
			return G_SMTP_STORAGEEXCEDED_STR;
		case G_SMTP_UNKNOWNUSER:
			return G_SMTP_UNKNOWNUSER_STR;
		case G_SMTP_FORWARDED:
			return G_SMTP_FORWARDED_STR;

		// *********************************************************************
		//                          HTTP error codes
		// *********************************************************************
		case G_HTTP_BAD_URL:
			return G_HTTP_BAD_URL_STR;
		case G_HTTP_TCP_FAILED:
			return G_HTTP_TCP_FAILED_STR;
		case G_HTTP_HOST_UNKNOWN:
			return G_HTTP_HOST_UNKNOWN_STR;
		case G_HTTP_TCP_CONNECT:
			return G_HTTP_TCP_CONNECT_STR;
		case G_HTTP_FILE_ERROR:
			return G_HTTP_FILE_ERROR_STR;
		case G_HTTP_INSMEMORY:
			return G_HTTP_INSMEMORY_STR;
		case G_HTTP_BAD_PARAM:
			return G_HTTP_BAD_PARAM_STR;
		case G_HTTP_OVERFLOW:
			return G_HTTP_OVERFLOW_STR;
		case G_HTTP_CANCELLED:
			return G_HTTP_CANCELLED_STR;
		case G_HTTP_NO_CONTENT:
			return G_HTTP_NO_CONTENT_STR;
		case G_HTTP_MOVED:
			return G_HTTP_MOVED_STR;
		case G_HTTP_BAD_REQUEST:
			return G_HTTP_BAD_REQUEST_STR;
		case G_HTTP_FORBIDDEN:
			return G_HTTP_FORBIDDEN_STR;
		case G_HTTP_NOT_FOUND:
			return G_HTTP_NOT_FOUND_STR;
		case G_HTTP_PROTOCOL_ERROR:
			return G_HTTP_PROTOCOL_ERROR_STR;
		case G_HTTP_UNDEFINED:
			return G_HTTP_UNDEFINED_STR;
		case G_HTTP_TIMEOUT:
			return G_HTTP_TIMEOUT_STR;
		default:
			return G_UNKNOWN_ERROR_STR;
	}
}

}
