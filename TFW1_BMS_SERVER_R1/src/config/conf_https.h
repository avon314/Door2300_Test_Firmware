/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: conf_https.h
* Description	: Compile-time selection of the server transport.
*				  USE_HTTPS = 1 : HTTPS (TLS 1.2, Mbed TLS 4.1)  - production default.
*				  USE_HTTPS = 0 : plain HTTP                     - lab server only.
*				  Set USE_HTTPS=0 only through the project symbols of a lab build;
*				  production configurations must not define it.
*****************************************************************************/
#ifndef CONF_HTTPS_H_
#define CONF_HTTPS_H_

#ifndef USE_HTTPS
#define USE_HTTPS		1
#endif

#if !USE_HTTPS
#warning "USE_HTTPS=0: plain-HTTP build - lab use only, do not ship"
#endif

/* TEST ONLY: use a fixed, compiled-in TLS entropy seed instead of the factory seed in
 * flash. Every boot then starts the RNG from the same value, so TLS keys are predictable.
 * For bench tests only - MUST be 0 in any build that leaves the lab. */
#ifndef MM_TLS_TEST_FIXED_SEED
#define MM_TLS_TEST_FIXED_SEED	1
#endif

#if USE_HTTPS && MM_TLS_TEST_FIXED_SEED
#warning "MM_TLS_TEST_FIXED_SEED=1: fixed TLS seed - test builds only, do not ship"
#endif

/* Request path used when the configured endpoint has no path. */
#define HTTP_API_PATH	"/api/method/avon_iot.apis.iot.endpoint"

/* ---------------------------------------------------------------------------
 * TFW1_BMS_SERVER_R1: periodic HTTPS POST (https_client.c)
 * This firmware has no endpoint configuration in EEPROM, so the endpoint, token
 * and poll interval are compile-time. Override them through the project
 * symbols (for example HTTPS_SERVER_ENDPOINT=\"https://192.168.1.10:8443/\"
 * for the tools/tls_test/post_echo.py bench server).
 * ------------------------------------------------------------------------- */
/* "[https://]host[:port][/path]" - see url_parse.c. The default public echo
 * server uses an RSA certificate, which profile B (this build) supports. */
#ifndef HTTPS_SERVER_ENDPOINT
#if USE_HTTPS
#define HTTPS_SERVER_ENDPOINT	/*"https://httpbin.org/post"*/"https://echo.free.beeceptor.com/post"
#else
#define HTTPS_SERVER_ENDPOINT	"http://httpbin.org/post"
#endif
#endif

/* Sent as "Authorization: token <value>". Public echo servers send request
 * headers back, so only ever use a dummy token with them (guide section 9). */
#ifndef HTTPS_AUTH_TOKEN
#define HTTPS_AUTH_TOKEN		"dummy-token"
#endif

/* Time between two POSTs. */
#ifndef HTTPS_POLL_INTERVAL_MS
#define HTTPS_POLL_INTERVAL_MS	10000u
#endif

#endif /* CONF_HTTPS_H_ */
