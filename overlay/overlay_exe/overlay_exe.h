/* Copyright (C) 2015, Mikkel Krautz <mikkel@krautz.dk>

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   - Neither the name of the Mumble Developers nor the names of its
     contributors may be used to endorse or promote products derived from this
     software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MUMBLE_OVERLAY_EXE_H_
#define MUMBLE_OVERLAY_EXE_H_

/// OverlayHelperError represents exit codes returned by the
/// overlay helper process (mumble_ol.exe) on Windows.
enum OverlayHelperError {
	/// The overlay helper process was not passed a magic
	/// number as its first and only argument.
	OVERLAY_HELPER_ERROR_EXE_MISSING_MAGIC_ARGUMENT  = -1,
	/// The magic number on the command line of the overlay
	/// helper process could not be converted to an integer.
	OVERLAY_HELPER_ERROR_EXE_INVALID_MAGIC_ARGUMENT  = -2,
	/// The magic number on the command line of the overlay
	/// helper did not match the built-in magic number of
	/// the helper process.
	OVERLAY_HELPER_ERROR_EXE_MAGIC_MISMATCH          = -3,
	/// The overlay helper process was unable to configure
	/// its environment in preparation of loading the
	/// overlay DLL.
	OVERLAY_HELPER_ERROR_EXE_CONFIGURE_ENVIRONMENT   = -4,
	/// The overlay helper process was unable to get the
	/// path to the overlay DLL.
	OVERLAY_HELPER_ERROR_EXE_GET_DLL_PATH            = -5,
	/// The overlay helper process was unable to load the
	/// overlay DLL.
	OVERLAY_HELPER_ERROR_EXE_LOAD_DLL                = -6,
	/// The overlay helper process was uanble to look up
	/// the 'OverlayHelperProcessMain' entry point in the
	/// overlay DLL.
	OVERLAY_HELPER_ERROR_EXE_LOOKUP_ENTRY_POINT      = -7,

	/// The magic number passed to the  overlay DLL's
	/// OverlayHelperProcessMain function did not match
	/// the overlay DLL's built-in magic number.
	OVERLAY_HELPER_ERROR_DLL_MAGIC_MISMATCH          = -1000,
	/// The overlay helper process exited due to an error
	/// in the Windows message loop.
	OVERLAY_HELPER_ERROR_DLL_MESSAGE_LOOP            = -1001,
};

/// OverlayHelperErrorToString converts an OverlayHelperError value
/// to a printable string representation.
static inline const char *OverlayHelperErrorToString(OverlayHelperError err) {
	#define OHE(x) case x: return #x
	switch (err) {
		OHE(OVERLAY_HELPER_ERROR_EXE_MISSING_MAGIC_ARGUMENT);
		OHE(OVERLAY_HELPER_ERROR_EXE_INVALID_MAGIC_ARGUMENT);
		OHE(OVERLAY_HELPER_ERROR_EXE_MAGIC_MISMATCH);
		OHE(OVERLAY_HELPER_ERROR_EXE_CONFIGURE_ENVIRONMENT);
		OHE(OVERLAY_HELPER_ERROR_EXE_GET_DLL_PATH);
		OHE(OVERLAY_HELPER_ERROR_EXE_LOAD_DLL);
		OHE(OVERLAY_HELPER_ERROR_EXE_LOOKUP_ENTRY_POINT);
		OHE(OVERLAY_HELPER_ERROR_DLL_MAGIC_MISMATCH);
		OHE(OVERLAY_HELPER_ERROR_DLL_MESSAGE_LOOP);
	}
	return NULL;
}

#endif
