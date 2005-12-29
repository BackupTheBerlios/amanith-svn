/****************************************************************************
** $file: amanith/gerror.h   0.2.0.0   edited Dec, 12 2005
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

#ifndef GERROR_H
#define GERROR_H

/*!
	\file gerror.h
	\brief Global error codes used by Amanith classes and functions.
*/

#include "gglobal.h"

namespace Amanith {

	// *********************************************************************
	//                          Generic error codes
	// *********************************************************************
	//! No Error / Operation has been completed successfully
	#define G_NO_ERROR					0
	//! Unknown and unexpected error
	#define G_UNKNOWN_ERROR				-199
	//! File was not found
	#define G_FILE_NOT_FOUND			-101
	//! File already exist
	#define G_FILE_ALREADY_EXISTS		-102
	//! Invalid format (ex: unsupported image file)
	#define G_INVALID_FORMAT			-103
	//! Permission denied
	#define G_PERMISSION_DENIED			-104
	//! Invalid parameter (maybe a NULL pointer, or wrong number)
	#define G_INVALID_PARAMETER			-105
	//! Invalid XML node
	#define G_INVALID_NODE				-106
	//! Passed ClassID or GElement is not supported for the specified operation
	#define G_UNSUPPORTED_CLASSID		-107
	//! Specified index is out of range
	#define G_OUT_OF_RANGE				-108
	//! Generic stream read error
	#define G_READ_ERROR				-109
	//! Generic stream write error
	#define G_WRITE_ERROR				-110
	//! Generic memory (allocation/deallocation) error
	#define G_MEMORY_ERROR				-111
	//! Class cannot do operation because a missing kernel owner
	#define G_MISSING_KERNEL			-112
	//! Missed feature
	#define G_MISSED_FEATURE			-113
	//! Entry already exists
	#define G_ENTRY_ALREADY_EXISTS		-114
	//! Invalid operation
	#define G_INVALID_OPERATION			-115

	// *********************************************************************
	//                        Plugin system error codes
	// *********************************************************************
	//! Plugin is not loaded
	#define G_PLUGIN_NOTLOADED			-201
	//! Plugin not present
	#define G_PLUGIN_NOTPRESENT			-202
	//! Plug symbol not resolved
	#define G_PLUGIN_SYMBOL_UNRESOLVED	-203
	//! Plugin is already loaded
	#define G_PLUGIN_ALREADYLOADED		-204
	//! Plugin has created some instances, so it can't be unload
	#define G_PLUGIN_ISTANCED			-205

	// *********************************************************************
	//                        Drawboard error codes
	// *********************************************************************
	//! Invalid cache bank.
	#define G_DRAWBOARD_INVALID_CACHEBANK	-301
	//! Cache not written
	#define G_DRAWBOARD_CACHE_NOT_WRITTEN	-302

	// *********************************************************************
	//                           SMTP error codes
	// *********************************************************************
	/*! Answer was not expected			 */
	#define G_SMTP_UNEXPECTEDANSWER		-2000
	/*! Service unavailable				 */
	#define G_SMTP_SERVICECLOSED		-2001
	/*! Host recognize but can't exec cmd*/
	#define G_SMTP_NOTIMPLEMENTED		-2002
	/*! Server doesn't support MIME ext. */
	#define G_SMTP_MIMENOTSUPPORTED		-2003
	/*! Refused by server				 */
	#define G_SMTP_SERVERCANTEXECUTE	-2004
	/*! Can not connect to the server	 */
	#define G_SMTP_CANTCONNECT			-2005
	/*! Error during communication		 */
	#define G_SMTP_DATAERROR			-2006
	/*! Bad parameters					 */
	#define G_SMTP_SYNTAXERROR			-2007
	/*! Server limits exceeded			 */
	#define G_SMTP_STORAGEEXCEDED		-2008
	/*! Unknown destinee				 */
	#define G_SMTP_UNKNOWNUSER			-2009
	/*! Address OK, unknown on this server*/
	#define G_SMTP_FORWARDED			-2010

	// *********************************************************************
	//                          HTTP error codes
	// *********************************************************************
	#define G_HTTP_BAD_URL				-1000	//!< Bad url
	#define G_HTTP_TCP_FAILED			-1001	//!< TCP failed
	#define G_HTTP_HOST_UNKNOWN			-1002	//!< Host unknown
	#define G_HTTP_TCP_CONNECT			-1003	//!< TCP connect
	#define G_HTTP_FILE_ERROR			-1004	//!< File error
	#define G_HTTP_INSMEMORY			-1005	//!< Insufficient memory
	#define G_HTTP_BAD_PARAM			-1006	//!< Bad parameters
	#define G_HTTP_OVERFLOW				-1007	//!< Overflow
	#define G_HTTP_CANCELLED			-1008	//!< Canceled
	#define G_HTTP_NO_CONTENT			-1009	//!< No content
	#define G_HTTP_MOVED				-1010	//!< Moved
	#define G_HTTP_BAD_REQUEST			-1011	//!< Bad request
	#define G_HTTP_FORBIDDEN			-1012	//!< Forbidden
	#define G_HTTP_NOT_FOUND			-1013	//!< Not found
	#define G_HTTP_PROTOCOL_ERROR		-1014	//!< Protocol error
	#define G_HTTP_UNDEFINED			-1015	//!< Undefined
	#define G_HTTP_TIMEOUT				-1016	//!< Timeout

	// *********************************************************************
	//                            Error utilities
	// *********************************************************************

	/*!
		\class ErrorUtils
		\brief This class implements some error utilities.
	*/
	class G_EXPORT ErrorUtils {
	public:
		/*!
			This function generate an error string corresponding to the given error code.\n
			The translation is based on the chosen language during Amanith compilation, defined
			by the global constant G_USED_LANG present in gglobal.h file.
		*/
		static GString ErrToString(const GError ErrorCode);
	};

};	// end namespace Amanith

#endif
