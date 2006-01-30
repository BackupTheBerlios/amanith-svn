/****************************************************************************
** $file: amanith/lang/gerror_en.h   0.3.0.0   edited Jan, 30 2006
**
** Italian error codes translation.
**
**
** Copyright (C) 2004-2006 Mazatech Inc. All rights reserved.
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

#ifndef GERROR_IT_H
#define GERROR_IT_H

/*!
	\file gerror_it.h
	\brief Italian error code translations.
*/

namespace Amanith {

	// *********************************************************************
	//                        Italian error codes translation
	// *********************************************************************
	//! No Error / Operation has been completed successfully
	#define G_NO_ERROR_STR				"Nessun errore"
	//! Unknown and unespected error
	#define G_UNKNOWN_ERROR_STR			"Error sconosciuto"
	//! File was not found
	#define G_FILE_NOT_FOUND_STR		"File non trovato"
	//! File already exist
	#define G_FILE_ALREADY_EXISTS_STR	"Il file è già esistente"
	//! Invalid format (ex: unsupported image file)
	#define G_INVALID_FORMAT_STR		"Formato non valido"
	//! Permission denied
	#define G_PERMISSION_DENIED_STR		"Permesso negato"
	//! Invalid parameter (maybe a NULL pointer, or wrong number)
	#define G_INVALID_PARAMETER_STR		"Parametro non valido"
	//! Invalid XML node
	#define G_INVALID_NODE_STR			"Nodo non valido"
	//! Passed ClassID or GElement is not supported for the specified operation
	#define G_UNSUPPORTED_CLASSID_STR	"ClassID non supportato"
	//! Specified index is out of range
	#define G_OUT_OF_RANGE_STR			"Intervallo violato"
	//! Generic stream read error
	#define G_READ_ERROR_STR			"Errore di lettura"
	//! Generic stream write error
	#define G_WRITE_ERROR_STR			"Errore di scrittura"
	//! Generic memory (allocation/deallocation) error
	#define G_MEMORY_ERROR_STR			"Errore di memoria (allocazione/deallocazione)"
	//! Class cannot do operation because a missing kernel owner
	#define G_MISSING_KERNEL_STR		"L'operazione non può essere eseguita, kernel owner mancante"
	//! Plugin missed feature
	#define G_MISSED_FEATURE_STR		"La funzione richiesta è assente o non ancora implementata"
	//! Entry already exists
	#define G_ENTRY_ALREADY_EXISTS_STR	"Voce/Elemento già presente"
	//! Invalid operation
	#define G_INVALID_OPERATION_STR		"Operazione non valida/non possibile"

	//! Plugin is not loaded
	#define G_PLUGIN_NOTLOADED_STR		"Plugin non caricato"
	//! Plugin not present
	#define G_PLUGIN_NOTPRESENT_STR		"Plugin non trovato/presente"
	//! Plug symbol not resolved
	#define G_PLUGIN_SYMBOL_UNRESOLVED_STR		"Simbolo non risolto/trovato"
	//! Plugin is already loaded	
	#define G_PLUGIN_ALREADYLOADED_STR	"Plugin già caricato/registrato"
	//! Plugin has created some instances, so it can't be unload
	#define G_PLUGIN_ISTANCED_STR		"Il plugin ha creato delle istanze e non può essere scaricato dalla memoria"

	// *********************************************************************
	//                           SMTP error codes
	// *********************************************************************
	/*! Answer was not expected			 */
	#define G_SMTP_UNEXPECTEDANSWER_STR		"Risposta inaspettata"
	/*! Service unavailable				 */
	#define G_SMTP_SERVICECLOSED_STR		"Servizio nond isponibile"
	/*! Host recognize but can't exec cmd*/
	#define G_SMTP_NOTIMPLEMENTED_STR		"Comando riconosciuto ma non implementato dall'host"
	/*! Server doesn't support MIME ext. */
	#define G_SMTP_MIMENOTSUPPORTED_STR		"Estensione MIME non supportata"
	/*! Refused by server				 */
	#define G_SMTP_SERVERCANTEXECUTE_STR	"Rifiuto di esecuzione"
	/*! Can not connect to the server	 */
	#define G_SMTP_CANTCONNECT_STR			"Impossibile connettersi"
	/*! Error during communication		 */
	#define G_SMTP_DATAERROR_STR			"Errore dati durante la comunicazione"
	/*! Bad parameters					 */
	#define G_SMTP_SYNTAXERROR_STR			"Parametri non validi, sintassi sbagliata"
	/*! Server limits exceeded			 */
	#define G_SMTP_STORAGEEXCEDED_STR		"Limiti di immagazzinamento superati"
	/*! Unknown destinee				 */
	#define G_SMTP_UNKNOWNUSER_STR			"Utente destinatario sconosciuto"
	/*! Address OK, unknown on this server*/
	#define G_SMTP_FORWARDED_STR			"Indirizzo corretto ma sconosciuto dal server (forward)"

	// *********************************************************************
	//                          HTTP error codes
	// *********************************************************************
	#define G_HTTP_BAD_URL_STR				"Indirizzo (url) non valido"
	#define G_HTTP_TCP_FAILED_STR			"Chiamata alla libreria di rete fallita"
	#define G_HTTP_HOST_UNKNOWN_STR			"Host sconosciuto"
	#define G_HTTP_TCP_CONNECT_STR			"Errore di connessione TCP al server HTTP"
	#define G_HTTP_FILE_ERROR_STR			"Errore di file"
	#define G_HTTP_INSMEMORY_STR			"Memoria insufficiente"
	#define G_HTTP_BAD_PARAM_STR			"Parametro non valido"
	#define G_HTTP_OVERFLOW_STR				"Buffer uetnte troppo piccolo (overflow)"
	#define G_HTTP_CANCELLED_STR			"Operazione cancellata"
	#define G_HTTP_NO_CONTENT_STR			"Nessun contenuto (dati vuoti)"
	#define G_HTTP_MOVED_STR				"Indirizzo spostato"
	#define G_HTTP_BAD_REQUEST_STR			"Richiesta non corretta"
	#define G_HTTP_FORBIDDEN_STR			"Accesso vietatao"
	#define G_HTTP_NOT_FOUND_STR			"Non trovato"
	#define G_HTTP_PROTOCOL_ERROR_STR		"Errore di protocollo"
	#define G_HTTP_UNDEFINED_STR			"Errore indefinitio"
	#define G_HTTP_TIMEOUT_STR				"Tempo concesso scaduto (timeout)"

};	// end namespace Amanith

#endif
