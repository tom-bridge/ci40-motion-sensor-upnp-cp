/***************************************************************************************************
 * Copyright (c) 2015, Imagination Technologies Limited
 * All rights reserved.
 *
 * Redistribution and use of the Software in source and binary forms, with or
 * without modification, are permitted provided that the following conditions are met:
 *
 *     1. The Software (including after any modifications that you make to it) must
 *        support the FlowCloud Web Service API provided by Licensor and accessible
 *        at  http://ws-uat.flowworld.com and/or some other location(s) that we specify.
 *
 *     2. Redistributions of source code must retain the above copyright notice, this
 *        list of conditions and the following disclaimer.
 *
 *     3. Redistributions in binary form must reproduce the above copyright notice, this
 *        list of conditions and the following disclaimer in the documentation and/or
 *        other materials provided with the distribution.
 *
 *     4. Neither the name of the copyright holder nor the names of its contributors may
 *        be used to endorse or promote products derived from this Software without
 *        specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 **************************************************************************************************/

/**
 * @file log.h
 * @brief Header file for logging.
 */

#ifndef LOG_H
#define LOG_H

#ifdef	__cplusplus
extern "C" {
#endif


#include <stdio.h>

/** Fatal error log level. */
#define LOG_FATAL (1)
/** Error log level. */
#define LOG_ERR (2)
/** Warning log level. */
#define LOG_WARN (3)
/** Information log level. */
#define LOG_INFO (4)
/** Debug log level. */
#define LOG_DBG (5)

/**	Macro for logging message at the specified level. */
#define LOG(level, ...) do {  \
                            if (level <= debug_level) \
                            { \
                                if (debug_level == LOG_DBG) \
                                    fprintf(stdout,"%s:%d: ", __FILE__, __LINE__); \
                                fprintf(stdout, __VA_ARGS__); \
                                fprintf(stdout, "\n\n"); \
                                fflush(stdout); \
                            } \
                        } while (0)

extern int debug_level;

#ifdef	__cplusplus
}
#endif

#endif	/* LOG_H */
