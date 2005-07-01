/****************************************************************************
** $file: amanith/lang/gerror_en.h   0.1.0.0   edited Jun 30 08:00
**
** English error codes translation.
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

#ifndef GERROR_EN_H
#define GERROR_EN_H

/*!
	\file gerror_en.h
	\brief English error code translations.
*/
namespace Amanith {

	// *********************************************************************
	//                        Italian error codes translation
	// *********************************************************************
	//! No Error / Operation has been completed successfully
	#define G_NO_ERROR_STR				"No error / successfully operation"
	//! Unknown and unespected error
	#define G_UNKNOWN_ERROR_STR			"Unknown error"
	//! File was not found
	#define G_FILE_NOT_FOUND_STR		"File not found"
	//! File already exist
	#define G_FILE_ALREADY_EXISTS_STR	"File already exists"
	//! Invalid format (ex: unsupported image file)
	#define G_INVALID_FORMAT_STR		"Invalid format"
	//! Permission denied
	#define G_PERMISSION_DENIED_STR		"Permission denied"
	//! Invalid parameter (maybe a NULL pointer, or wrong number)
	#define G_INVALID_PARAMETER_STR		"Invalid parameter"
	//! Invalid XML node
	#define G_INVALID_NODE_STR			"Invalid node"
	//! Passed ClassID or GElement is not supported for the specified operation
	#define G_UNSUPPORTED_CLASSID_STR	"Unsupported ClassID"
	//! Specified index is out of range
	#define G_OUT_OF_RANGE_STR			"Out of range"
	//! Generic stream read error
	#define G_READ_ERROR_STR			"Read error"
	//! Generic stream write error
	#define G_WRITE_ERROR_STR			"Write error"
	//! Generic memory (allocation/deallocation) error
	#define G_MEMORY_ERROR_STR			"Memory (allocation/deallocation) error"
	//! Class cannot do operation because a missing kernel owner
	#define G_MISSING_KERNEL_STR		"Class cannot do operation because a missing kernel owner"
	//! Plugin missed feature
	#define G_MISSED_FEATURE_STR		"Feature is missed or not yet implemented"
	//! Entry already exists
	#define G_ENTRY_ALREADY_EXISTS_STR	"Entry/Element already exists"
	//! Invalid operation
	#define G_INVALID_OPERATION_STR		"Invalid/impossibile operation"

	//! Plugin is not loaded
	#define G_PLUGIN_NOTLOADED_STR		"Plugin has not been loaded"
	//! Plugin not present
	#define G_PLUGIN_NOTPRESENT_STR		"Plugin not found/present"
	//! Plug symbol not resolved
	#define G_PLUGIN_SYMBOL_UNRESOLVED_STR		"Symbol was not resolved/found"
	//! Plugin is already loaded	
	#define G_PLUGIN_ALREADYLOADED_STR	"Plugin is already loaded/registered"
	//! Plugin has created some instances, so it can't be unload
	#define G_PLUGIN_ISTANCED_STR		"Plugin has created some instances, so it can't be unload form memory"


	// *********************************************************************
	//                           SMTP error codes
	// *********************************************************************
	/*! Answer was not expected			 */
	#define G_SMTP_UNEXPECTEDANSWER_STR		"Answer was not expected	"
	/*! Service unavailable				 */
	#define G_SMTP_SERVICECLOSED_STR		"Service unavailable"
	/*! Host recognize but can't exec cmd*/
	#define G_SMTP_NOTIMPLEMENTED_STR		"Host recognize but can't exec cmd"
	/*! Server doesn't support MIME ext. */
	#define G_SMTP_MIMENOTSUPPORTED_STR		"Server doesn't support MIME extension"
	/*! Refused by server				 */
	#define G_SMTP_SERVERCANTEXECUTE_STR	"Refused by server"
	/*! Can not connect to the server	 */
	#define G_SMTP_CANTCONNECT_STR			"I cannot connect to the server"
	/*! Error during communication		 */
	#define G_SMTP_DATAERROR_STR			"Error during communication"
	/*! Bad parameters					 */
	#define G_SMTP_SYNTAXERROR_STR			"Bad parameters / syntax error"
	/*! Server limits exceeded			 */
	#define G_SMTP_STORAGEEXCEDED_STR		"Server storage limits exceeded"
	/*! Unknown destinee				 */
	#define G_SMTP_UNKNOWNUSER_STR			"Unknown destinee"
	/*! Address OK, unknown on this server*/
	#define G_SMTP_FORWARDED_STR			"Address OK, unknown on this server (forward)"

	// *********************************************************************
	//                          HTTP error codes
	// *********************************************************************
	#define G_HTTP_BAD_URL_STR				"Url does not conform to http protocol"
	#define G_HTTP_TCP_FAILED_STR			"Call to network library failed"
	#define G_HTTP_HOST_UNKNOWN_STR			"Cannot resolve host address"
	#define G_HTTP_TCP_CONNECT_STR			"Can't reach the http server"
	#define G_HTTP_FILE_ERROR_STR			"File stream error"
	#define G_HTTP_INSMEMORY_STR			"Insufficient memory"
	#define G_HTTP_BAD_PARAM_STR			"Bad parameter in the function call"
	#define G_HTTP_OVERFLOW_STR				"User buffer so little (overflow)"
	#define G_HTTP_CANCELLED_STR			"Transfer aborted"
	#define G_HTTP_NO_CONTENT_STR			"Data is empty"
	#define G_HTTP_MOVED_STR				"Request moved to other server"
	#define G_HTTP_BAD_REQUEST_STR			"Bad request received by the server"
	#define G_HTTP_FORBIDDEN_STR			"Request forbidden"
	#define G_HTTP_NOT_FOUND_STR			"Destination file not found"
	#define G_HTTP_PROTOCOL_ERROR_STR		"Protocol error"
	#define G_HTTP_UNDEFINED_STR			"Undefined error"
	#define G_HTTP_TIMEOUT_STR				"timeout in TCP dialog"

};	// end namespace Amanith

#endif
